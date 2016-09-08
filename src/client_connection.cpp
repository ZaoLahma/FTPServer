/*
 * client_connection.cpp
 *
 *  Created on: Sep 1, 2016
 *      Author: janne
 */

#include "../inc/client_connection.h"
#include "../inc/admin_interface_events.h"
#include "../inc/thread_fwk/jobdispatcher.h"
#include "../inc/server_socket_listener.h"
#include "../inc/retr_job.h"
#include "../inc/stor_job.h"
#include "../inc/ftp_thread_model.h"
#include "../inc/ftp_utils.h"
#include "../inc/data_handling_events.h"
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

ClientConnection::ClientConnection(int32_t _controlFd, ConfigHandler& _configHandler) :
active(false),
controlFd(_controlFd),
dataFd(-1),
user(nullptr),
configHandler(_configHandler),
loggedIn(false),
binaryFlag(false),
inactiveTooLong(false),
disconnected(false),
noOfCycles(0) {
	JobDispatcher::GetApi()->SubscribeToEvent(controlFd, this);

	std::string initConnStr = "220 OK.";

	FTPUtils::SendString(initConnStr, controlFd, socketApi);
}

ClientConnection::~ClientConnection() {

	JobDispatcher::GetApi()->UnsubscribeToEvent(controlFd, this);

	/*
	 * Cancel all ongoing file transfers
	 */
	HandleAborCommand();
}

void ClientConnection::HandleEvent(const uint32_t eventNo, const EventDataBase* dataPtr) {

	if((int32_t) eventNo != controlFd) {
		/*
		 * Severe fault. Started sending stuff to
		 * the wrong client connection potentially.
		 *
		 * End execution.
		 */
		JobDispatcher::GetApi()->Log("Client connection %d received event for %d. dataPtr: %p Ending execution", controlFd, eventNo, dataPtr);
		JobDispatcher::GetApi()->NotifyExecutionFinished();
	}

	active = true;
	inactiveTooLong = false;

	FTPCommand command = GetCommand();
	if(false == loggedIn) {
		switch(command.ftpCommand) {
			case FTPCommandEnum::USER: {
				HandleUserCommand(command);
			}
			break;
			case FTPCommandEnum::PASS: {
				HandlePassCommand(command);
			}
			break;
			case FTPCommandEnum::QUIT: {
				HandleQuitCommand();
			}
			case FTPCommandEnum::NOT_IMPLEMENTED: {
				std::string send_string = "500 - Not implemented";
				FTPUtils::SendString(send_string, controlFd, socketApi);
			}
			break;
			default:
				break;
		}
	} else {
		switch(command.ftpCommand) {
			case FTPCommandEnum::PORT: {
				HandlePortCommand(command);
			}
			break;
			case FTPCommandEnum::LIST: {
				HandleListCommand(command);
			}
			break;
			case FTPCommandEnum::QUIT: {
				HandleQuitCommand();
			}
			break;
			case FTPCommandEnum::PWD: {
				HandlePwdCommand();
			}
			break;
			case FTPCommandEnum::RETR: {
				HandleRetrCommand(command);
			}
			break;
			case FTPCommandEnum::TYPE: {
				HandleTypeCommand(command);
			}
			break;
			case FTPCommandEnum::STOR: {
				HandleStorCommand(command);
			}
			break;
			case FTPCommandEnum::DELE: {
				HandleDeleCommand(command);
			}
			break;
			case FTPCommandEnum::ABOR: {
				HandleAborCommand();
			}
			break;
			case FTPCommandEnum::CWD: {
				HandleCwdCommand(command);
			}
			break;
			case FTPCommandEnum::MKD: {
				HandleMkdCommand(command);
			}
			break;
			case FTPCommandEnum::RMD: {
				HandleRmdCommand(command);
			}
			break;
			case FTPCommandEnum::NOT_IMPLEMENTED: {
				std::string send_string = "500 - Not implemented";
				FTPUtils::SendString(send_string, controlFd, socketApi);
			}
			break;
			default:
				break;
		}
	}

	active = false;
}

FTPCommand ClientConnection::GetCommand() {
	FTPCommand retVal;
	retVal.ftpCommand = FTPCommandEnum::NOT_IMPLEMENTED;
	retVal.args = "";

	std::vector<std::string> command = SplitString(GetControlChannelData(), " ");

	if(command[0] == "USER") {
		retVal.ftpCommand = FTPCommandEnum::USER;
		retVal.args = command[1];
	} else if(command[0] == "PASS") {
		retVal.ftpCommand = FTPCommandEnum::PASS;
		retVal.args = command[1];
	} else if(command[0] == "QUIT") {
		retVal.ftpCommand = FTPCommandEnum::QUIT;
	} else if(command[0] == "PWD") {
		retVal.ftpCommand = FTPCommandEnum::PWD;
	} else if(command[0] == "PORT") {
		retVal.ftpCommand = FTPCommandEnum::PORT;
		retVal.args = command[1];
	} else if(command[0] == "LIST") {
		retVal.ftpCommand = FTPCommandEnum::LIST;
		if(command.size() > 1) {
			retVal.args = command[1];
		}
	} else if(command[0] == "RETR") {
		retVal.ftpCommand = FTPCommandEnum::RETR;
		retVal.args = command[1];
	} else if(command[0] == "STOR") {
		retVal.ftpCommand = FTPCommandEnum::STOR;
		retVal.args = command[1];
	} else if(command[0] == "DELE") {
		retVal.ftpCommand = FTPCommandEnum::DELE;
		retVal.args = command[1];
	} else if(command[0] == "TYPE") {
		retVal.ftpCommand = FTPCommandEnum::TYPE;
		retVal.args = command[1];
	} else if(command[0] == "CWD") {
		retVal.ftpCommand = FTPCommandEnum::CWD;
		retVal.args = command[1];
	} else if(command[0] == "MKD") {
		retVal.ftpCommand = FTPCommandEnum::MKD;
		retVal.args = command[1];
	} else if(command[0] == "RMD") {
		retVal.ftpCommand = FTPCommandEnum::RMD;
		retVal.args = command[1];
	} else if(command[0].find("ABOR") != std::string::npos) {
		retVal.ftpCommand = FTPCommandEnum::ABOR;
	} else {
		JobDispatcher::GetApi()->Log("Command: %s not implemented", command[0].c_str());
	}

	return retVal;
}

std::string ClientConnection::GetControlChannelData() {
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

	return stringBuf;
}

std::vector<std::string> ClientConnection::SplitString(const std::string& str,
													   const std::string& delimiter) {
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

void ClientConnection::HandleUserCommand(const FTPCommand& command) {
	user = configHandler.GetUser(command.args);

	std::string send_string = "330 OK, send password";
	FTPUtils::SendString(send_string, controlFd, socketApi);
}

void ClientConnection::HandlePassCommand(const FTPCommand& command) {
	std::string send_string = "530, user/passwd not correct";
	if(nullptr != user) {
		if(command.args == user->passwd) {
			send_string = "230 OK, user " + user->userName + " logged in";
			currDir = user->homeDir;
			ftpRootDir = user->homeDir;
			loggedIn = true;
		} else {

		}
	}
	FTPUtils::SendString(send_string, controlFd, socketApi);
}

void ClientConnection::HandlePortCommand(const FTPCommand& command) {
	std::string send_string = "200 PORT command successful";
	std::vector<std::string> addressInfo = SplitString(command.args, ",");
	uint32_t clientPort = std::stoi(addressInfo[4]) * 256 + std::stoi(addressInfo[5]);

	std::string clientAddress = addressInfo[0] + "." +
								addressInfo[1] + "." +
								addressInfo[2] + "." +
								addressInfo[3];

	dataFd = socketApi.getClientSocketFileDescriptor(clientAddress, std::to_string(clientPort));

	JobDispatcher::GetApi()->Log("dataFd: %d", dataFd);
	FTPUtils::SendString(send_string, controlFd, socketApi);
}

void ClientConnection::HandleListCommand(const FTPCommand& command) {
	char buffer[2048];
	std::string response = "";
	std::string ls = "ls -l";
	ls.append(" ");
	if(command.args == "") {
		ls.append(currDir);
	} else {
		ls.append(command.args);
	}
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

	std::string send_string = "150 LIST executed ok, data follows";
	FTPUtils::SendString(send_string, controlFd, socketApi);

	FTPUtils::SendString(response, dataFd, socketApi);

	send_string = "226 LIST data send finished";
	FTPUtils::SendString(send_string, controlFd, socketApi);

	socketApi.disconnect(dataFd);
	dataFd = -1;
}

void ClientConnection::HandleRetrCommand(const FTPCommand& command) {
	std::string filePath = currDir + "/" + command.args;
	JobDispatcher::GetApi()->ExecuteJobInGroup(new RetrJob(filePath, dataFd, controlFd, binaryFlag), DATA_CHANNEL_THREAD_GROUP_ID);
}

void ClientConnection::HandleStorCommand(const FTPCommand& command) {
	if(user->rights == WRITE) {
		std::string filePath = currDir + "/" + command.args;
		JobDispatcher::GetApi()->ExecuteJobInGroup(new StorJob(filePath, dataFd, controlFd, binaryFlag), DATA_CHANNEL_THREAD_GROUP_ID);
	} else {
		std::string send_string = "550 STOR refused due to user access rights";
		FTPUtils::SendString(send_string, controlFd, socketApi);
	}
}

void ClientConnection::HandleDeleCommand(const FTPCommand& command) {
	std::string send_string = "";
	if(user->rights == WRITE) {
		int status;
		std::string fileString = currDir + "/" + command.args;
		status = remove(fileString.c_str());
		if(0 == status) {
			send_string = "250 " + fileString + " deleted OK";
		} else {
			send_string = "550 DELE of " + fileString + " not performed due to unknown reason"; //FIXME
		}
	} else {
		send_string = "550 DELE refused due to user access rights";
	}

	FTPUtils::SendString(send_string, controlFd, socketApi);
}

void ClientConnection::HandleAborCommand() {
	JobDispatcher::GetApi()->RaiseEvent(ABORT_DATA_TRANSFER_EVENT_ID, new AbortDataTransferData(dataFd));
}

void ClientConnection::HandleTypeCommand(const FTPCommand& command) {
	if(command.args == "I" || command.args == "A") {
		std::string send_string = "200 Transfer mode changed to: " + command.args;
		if(command.args == "I") {
			binaryFlag = true;
		} else {
			binaryFlag = false;
		}
		FTPUtils::SendString(send_string, controlFd, socketApi);
	} else {
		std::string send_string = "500 - Not implemented";
		FTPUtils::SendString(send_string, controlFd, socketApi);
	}
}

void ClientConnection::HandleQuitCommand() {
	std::string send_string = "221 Bye Bye";
	FTPUtils::SendString(send_string, controlFd, socketApi);

	disconnected = true;

	JobDispatcher::GetApi()->UnsubscribeToEvent(controlFd, this);
	JobDispatcher::GetApi()->RaiseEvent(CLIENT_DISCONNECTED_EVENT, new ClientStatusChangeEventData(controlFd));
}

void ClientConnection::HandlePwdCommand() {
	JobDispatcher::GetApi()->RaiseEvent(ABORT_DATA_TRANSFER_EVENT_ID, new AbortDataTransferData(dataFd));

	std::string send_string = "257 \"" + currDir + "\"";
	FTPUtils::SendString(send_string, controlFd, socketApi);
}

void ClientConnection::HandleCwdCommand(const FTPCommand& command) {
	std::string send_string = "250 CWD OK";

	std::vector<std::string> splitCurrDir = SplitString(currDir, "/");

	std::vector<std::string> changeDir;

	if(".." == command.args) {
		splitCurrDir.pop_back();
	} else {
		if(command.args.find("/") != std::string::npos) {
			changeDir = SplitString(command.args, "/");
			for(unsigned int i = 0; i < changeDir.size(); ++i) {
				if(changeDir[i] == ".") {

				} else if(changeDir[i] == "..") {
					splitCurrDir.pop_back();
				} else {
					splitCurrDir.push_back(changeDir[i]);
				}
			}
		} else {
			splitCurrDir.push_back(command.args);
		}
	}

	std::string tmpDir;

	for(unsigned int i = 0; i < splitCurrDir.size(); ++i) {
		tmpDir += "/" + splitCurrDir[i];
	}

	if(tmpDir.find(ftpRootDir) != std::string::npos) {

		char buffer[2048];
		std::string ls = "ls -l";
		ls.append(" " + tmpDir);
		ls.append(" 2>&1");
		std::string resString;
		FILE* file = popen(ls.c_str(), "r");
		while (!feof(file)) {
			if (fgets(buffer, 2048, file) != NULL) {
				resString.append(buffer);
			}
		}
		if(resString.find("No such file or directory") == std::string::npos) {
			currDir = tmpDir;
		} else {
			send_string = "550 CWD not performed. " + tmpDir + " - No such file or directory";
		}
	} else {
		send_string = "550 CWD outside of FTP root dir not allowed";
	}

	FTPUtils::SendString(send_string, controlFd, socketApi);
}

void ClientConnection::HandleMkdCommand(const FTPCommand& command) {
	std::string send_string;
	if(user->rights == WRITE) {
		int status;
		std::string dirString = currDir + "/" + command.args;
		status = mkdir(dirString.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH | S_IWOTH);
		if(0 == status) {
			send_string = "257 " + dirString + " created OK";
		} else {
			send_string = "550 MKD of " + dirString + " not performed due to unknown reason"; //FIXME
		}
	} else {
		send_string = "550 MKD refused due to user rights";
	}

	FTPUtils::SendString(send_string, controlFd, socketApi);
}

void ClientConnection::HandleRmdCommand(const FTPCommand& command) {
	std::string send_string;
	if(user->rights == WRITE) {
		int status;
		std::string dirString = currDir + "/" + command.args;
		status = remove(dirString.c_str());
		if(0 == status) {
			send_string = "250 " + dirString + " removed OK";
		} else {
			send_string = "550 RMD of " + dirString + " not performed due to unknown reason"; //FIXME
		}
	} else {
		send_string = "550 RMD refused due to user access rights";
	}
	FTPUtils::SendString(send_string, controlFd, socketApi);
}

void ClientConnection::CheckIfInactive() {
	noOfCycles++;
	if(noOfCycles > 2 * 60 * 5) {
		noOfCycles = 0;
		if(true == inactiveTooLong) {
			HandleQuitCommand();
		} else {
			inactiveTooLong = true;
		}
	}
}

void ClientConnection::ForceDisconnect() {
	HandleQuitCommand();
}

bool ClientConnection::IsDisconnected() {
	return disconnected;
}
