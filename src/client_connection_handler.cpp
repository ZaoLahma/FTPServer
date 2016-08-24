/*
 * client_connection_handler.cpp
 *
 *  Created on: Aug 20, 2016
 *      Author: janne
 */

#include "../inc/client_connection_handler.h"
#include "../inc/thread_fwk/jobdispatcher.h"
#include "../inc/server_socket_listener.h"
#include <string>
#include <cstdlib>
#include <stdio.h>
#include <fstream>
#include <unistd.h>
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>

ClientConnectionHandler::~ClientConnectionHandler() {
	std::lock_guard<std::mutex> execLock(execMutex);
	delete user;
	if (-1 != dataFd) {
		socketApi.disconnect(dataFd);
	}
}

ClientConnectionHandler::ClientConnectionHandler(int fileDescriptor,
		ConfigHandler& config) :
		active(false), invalid(false), controlFd(fileDescriptor), ftpDir(""), currDir(
				ftpDir), dataFd(-1), transferMode("A"), transferActive(false), config(
				config), user(nullptr) {

	JobDispatcher::GetApi()->SubscribeToEvent(fileDescriptor, this);

	std::string initConnStr = "220 OK.\r\n";

	SendResponse(initConnStr, controlFd);
}

void ClientConnectionHandler::HandleEvent(unsigned int eventNo,
		const EventDataBase* dataPtr) {
	std::lock_guard<std::mutex> execLock(execMutex);

	if (true == invalid) {
		return;
	}

	HandleControlMessage();

	active = false;
	JobDispatcher::GetApi()->RaiseEvent(CLIENT_INACTIVE_EVENT,
			new ClientStatusChangeEventData(eventNo));
}

void ClientConnectionHandler::HandleControlMessage() {
	std::vector<std::string> command = GetCommand();

	if ("USER" == command[0]) {
		user = config.GetUser(command[1]);

		std::string send_string = "330 OK, send password\r\n";
		SendResponse(send_string, controlFd);
	} else if ("PASS" == command[0]) {
		std::string send_string =
				"530, user/passwd not correct or ftp directory not configured right";
		if (user != nullptr) {
			if (user->passwd == command[1]) {
				send_string = "230 OK, user logged in";
				ftpDir = user->homeDir;
				currDir = ftpDir;
			}
		}
		send_string += "\r\n";
		SendResponse(send_string, controlFd);
	} else if ("PWD" == command[0]) {
		std::string send_string = "257 \"" + currDir + "\"\r\n";
		SendResponse(send_string, controlFd);
	} else if ("PORT" == command[0]) {
		std::vector<std::string> addressInfo = SplitString(command[1], ",");
		std::string ipAddress = addressInfo[0] + "." + addressInfo[1] + "."
				+ addressInfo[2] + "." + addressInfo[3];
		unsigned int portNoInt = atoi(addressInfo[4].c_str()) * 256
				+ atoi(addressInfo[5].c_str());
		std::string portNo = std::to_string(portNoInt);

		dataFd = socketApi.getClientSocketFileDescriptor(ipAddress, portNo);

		std::string send_string = "200, PORT command ok\r\n";
		SendResponse(send_string, controlFd);
	} else if ("LIST" == command[0]) {
		char buffer[2048];
		std::string response = "";
		std::string ls = "ls -l";
		ls.append(" " + currDir);
		ls.append(" 2>&1");
		FILE* file = popen(ls.c_str(), "r");

		while (!feof(file)) {
			if (fgets(buffer, 2048, file) != NULL) {
				response.append(buffer);
			}
		}

		std::vector<std::string> responseVector = SplitString(response, "\n");
		response = "";
		for (unsigned int i = 0; i < responseVector.size(); ++i) {
			response += responseVector[i] + "\r\n";
		}

		response += "\r\n";

		std::string send_string = "150 LIST executed ok, data follows\r\n";
		SendResponse(send_string, controlFd);

		SendResponse(response, dataFd);

		send_string = "226 LIST data send finished\r\n";
		SendResponse(send_string, controlFd);

		socketApi.disconnect(dataFd);
		dataFd = -1;
	} else if ("RETR" == command[0]) {
		std::string filePath = currDir + "/" + command[1];

		std::ifstream fileStream(filePath.c_str(), std::ifstream::binary);
		fileStream.seekg(0, fileStream.end);
		int length = fileStream.tellg();
		fileStream.seekg(0, fileStream.beg);

		std::string send_string = "150 RETR executed ok, data follows\r\n";
		if ("A" == transferMode) {
			send_string =
					"150 RETR ok. Note transmission mode ASCII is slow, data follows\r\n";
		}
		SendResponse(send_string, controlFd);

		SocketBuf sendBuf;
		if ("A" == transferMode) {
			sendBuf.dataSize = 1;
			sendBuf.data = new char[1];
			transferActive = true;
			while (length > 0 && transferActive) {
				char c = fileStream.get();
				if (c == '\n') {
					*sendBuf.data = '\r';
					socketApi.sendData(dataFd, sendBuf);
				}
				*sendBuf.data = c;
				socketApi.sendData(dataFd, sendBuf);
				length -= 1;
				if (true == CheckControlChannel()) {
					HandleControlMessage();
				}
			}
		} else if ("I" == transferMode) {
			unsigned int max_buf = 2048;
			sendBuf.data = new char[max_buf];
			transferActive = true;
			while (length > 0 && transferActive) {
				unsigned int read_bytes = 0;
				while (read_bytes != max_buf) {
					sendBuf.data[read_bytes] = fileStream.get();
					read_bytes++;
					length -= 1;
					if (length == 0) {
						break;
					}
				}
				sendBuf.dataSize = read_bytes;
				socketApi.sendData(dataFd, sendBuf);
				if (true == CheckControlChannel()) {
					HandleControlMessage();
				}
			}
		}
		fileStream.close();
		delete[] sendBuf.data;

		send_string = "226 RETR data send finished\r\n";
		SendResponse(send_string, controlFd);

		socketApi.disconnect(dataFd);
		dataFd = -1;
	} else if ("CWD" == command[0]) {
		std::string tmpDir = "";
		std::string resString = "";
		bool validDirectory = true;
		if (command[1].find("..") != std::string::npos) {
			std::vector<std::string> levels = SplitString(command[1], "..");
			std::vector<std::string> splitDirectory = SplitString(currDir, "/");
			for (unsigned int i = 0; i < splitDirectory.size() - levels.size();
					++i) {
				tmpDir += '/' + splitDirectory[i];
			}
			if (tmpDir.find(ftpDir) != std::string::npos) {

			} else {
				resString = "Not allowed to CWD outside of FTP dir";
				validDirectory = false;
			}
		} else {
			char buffer[2048];
			std::string ls = "ls -l";
			ls.append(" " + currDir + "/" + command[1]);
			ls.append(" 2>&1");
			FILE* file = popen(ls.c_str(), "r");
			while (!feof(file)) {
				if (fgets(buffer, 2048, file) != NULL) {
					resString.append(buffer);
				}
			}
			if (resString.find("No such file or directory")
					== std::string::npos) {
				tmpDir = currDir + "/" + command[1];
			} else {
				validDirectory = false;
				resString = currDir + "/" + command[1]
						+ " - No such file or directory";
			}
		}
		if (validDirectory) {
			currDir = tmpDir;
			std::string send_string = "250 CWD ok\r\n";
			SendResponse(send_string, controlFd);
		} else {
			std::string send_string = "550 CWD not performed. " + resString
					+ "\r\n";
			SendResponse(send_string, controlFd);
		}
	} else if ("TYPE" == command[0]
			&& ("I" == command[1] || "A" == command[1])) {
		transferMode = command[1];
		std::string send_string = "200 Transfer mode change to: " + transferMode
				+ "\r\n";
		SendResponse(send_string, controlFd);
	} else if ("STOR" == command[0]) {
		std::string send_string =
				"550 STORE refused due to user access rights\r\n";
		if (user->rights == READ) {
			SendResponse(send_string, controlFd);
			socketApi.disconnect(dataFd);
			dataFd = -1;
		} else {
			std::ofstream fileStream(command[1]);

			std::string send_string =
					"150 STORE ok, send data pretty please\r\n";
			if ("A" == transferMode) {
				send_string =
						"150 STORE ok, note transmission mode ASCII is slow\r\n";
			}
			SendResponse(send_string, controlFd);

			SocketBuf receiveBuf;
			if ("A" == transferMode) {
				receiveBuf = socketApi.receiveData(dataFd, 1);
				transferActive = true;
				do {
					receiveBuf = socketApi.receiveData(dataFd, 1);
					if (0 != receiveBuf.dataSize && *receiveBuf.data != '\r') {
						fileStream << *receiveBuf.data;
					}
					delete[] receiveBuf.data;
					if (true == CheckControlChannel()) {
						HandleControlMessage();
					}
				} while (receiveBuf.dataSize != 0 && transferActive);
			} else if ("I" == transferMode) {
				transferActive = true;
				unsigned int max_buf = 2048;
				do {
					receiveBuf = socketApi.receiveData(dataFd, max_buf);
					fileStream.write(receiveBuf.data, receiveBuf.dataSize);
					delete[] receiveBuf.data;
					if (true == CheckControlChannel()) {
						HandleControlMessage();
					}
				} while (receiveBuf.dataSize == max_buf && transferActive);
			}

			send_string = "226 STOR data received ok\r\n";
			SendResponse(send_string, controlFd);

			socketApi.disconnect(dataFd);
		}

	} else if (command[0] == "MKD") {
		std::string send_string = "";
		if(user->rights == WRITE) {
			int status;
			std::string dirString = currDir + "/" + command[1];
			status = mkdir(dirString.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH | S_IWOTH);
			if(0 == status) {
				send_string = "257 " + dirString + " created OK\r\n";
			} else {
				send_string = "550 STORE not performed due to unknown reason\r\n"; //FIXME
			}
		} else {
			send_string = "550 STORE refused due to user access rights\r\n";
		}

		SendResponse(send_string, controlFd);

	} else if (command[0] == "RMD") {
		std::string send_string = "";
		if(user->rights == WRITE) {
			int status;
			std::string dirString = currDir + "/" + command[1];
			status = rmdir(dirString.c_str());
			if(0 == status) {
				send_string = "250 " + dirString + " removed OK\r\n";
			} else {
				send_string = "550 RMD not performed due to unknown reason\r\n"; //FIXME
			}
		} else {
			send_string = "550 RMD refused due to user access rights\r\n";
		}

		SendResponse(send_string, controlFd);
	} else if (command[0].find("ABOR") != std::string::npos) {
		transferActive = false;
		std::string send_string = "426 Abort ok\r\n";
		SendResponse(send_string, controlFd);
	} else if ("QUIT" == command[0]) {
		std::string send_string = "221 Bye Bye\r\n";
		SendResponse(send_string, controlFd);

		JobDispatcher::GetApi()->UnsubscribeToEvent(controlFd, this);
		JobDispatcher::GetApi()->RaiseEvent(CLIENT_DISCONNECTED_EVENT,
				new ClientStatusChangeEventData(controlFd));
		invalid = true;
	} else {
		std::string send_string = "500 Not implemented\r\n";
		SendResponse(send_string, controlFd);
	}
}

void ClientConnectionHandler::SendResponse(const std::string& response,
		int fileDescriptor) {
	SocketBuf sendData;
	sendData.dataSize = strlen(response.c_str());
	sendData.data = new char[sendData.dataSize];

	memcpy(sendData.data, response.c_str(), sendData.dataSize);

	printf("Sending %s\n", response.c_str());

	socketApi.sendData(fileDescriptor, sendData);

	delete[] sendData.data;
}

std::vector<std::string> ClientConnectionHandler::SplitString(
		const std::string& str, const std::string& delimiter) {
	std::vector<std::string> retVal;

	std::string::size_type start_pos = 0;
	std::string::size_type end_pos = str.find(delimiter);

	while (end_pos != std::string::npos) {
		if (start_pos != end_pos) {
			retVal.push_back(str.substr(start_pos, end_pos - start_pos));
		}
		start_pos = ++end_pos;
		end_pos = str.find(delimiter, end_pos);
	}

	if (end_pos == std::string::npos) {
		retVal.push_back(str.substr(start_pos, str.length()));
	}

	return retVal;
}

bool ClientConnectionHandler::CheckControlChannel() {
	fd_set rfds;
	FD_ZERO(&rfds);

	FD_SET(controlFd, &rfds);

	timeval timeout;
	timeout.tv_usec = 0;
	timeout.tv_sec = 0;
	int retval = select(controlFd + 1, &rfds, NULL, NULL, &timeout);
	if (retval > 0) {
		return true;
	}
	return false;
}

std::vector<std::string> ClientConnectionHandler::GetCommand() {
	SocketBuf buf;
	buf.data = nullptr;
	buf.dataSize = 0;

	std::string stringBuf;
	buf = socketApi.receiveData(controlFd, 1);

	while (*buf.data != '\n') {
		if (buf.dataSize != 0 && *buf.data != '\r') {
			stringBuf.append(buf.data, 1);
		}
		delete buf.data;

		buf = socketApi.receiveData(controlFd, 1);
	}

	if (nullptr != buf.data) {
		delete buf.data;
	}

	std::vector<std::string> command = SplitString(stringBuf, " ");

	if (command.size() == 0) {
		JobDispatcher::GetApi()->Log("ERROR: Couldn't parse command: %s",
				stringBuf.c_str());
		JobDispatcher::GetApi()->NotifyExecutionFinished();
	}

	return command;
}
