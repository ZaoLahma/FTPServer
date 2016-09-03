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
#include <dirent.h>

ClientConnection::ClientConnection(int32_t _controlFd, ConfigHandler& _configHandler) :
active(false),
controlFd(_controlFd),
dataFd(-1),
user(nullptr),
configHandler(_configHandler),
loggedIn(false) {
	JobDispatcher::GetApi()->SubscribeToEvent(controlFd, this);

	std::string initConnStr = "220 OK.";

	SendResponse(initConnStr, controlFd);
}

void ClientConnection::HandleEvent(const uint32_t eventNo, const EventDataBase* dataPtr) {
	active = true;

	if(eventNo != (uint32_t)controlFd) {
		active = false;
		return;
	}

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
		case FTPCommandEnum::NOT_IMPLEMENTED: {
			std::string send_string = "500 - Not implemented";
			SendResponse(send_string, controlFd);
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
			case FTPCommandEnum::NOT_IMPLEMENTED: {
				std::string send_string = "500 - Not implemented";
				SendResponse(send_string, controlFd);
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

void ClientConnection::SendResponse(const std::string& response, int fileDescriptor) {
	std::string stringToSend = response;
	stringToSend += "\r\n";
	SocketBuf sendData;
	sendData.dataSize = strlen(stringToSend.c_str());
	sendData.data = new char[sendData.dataSize];

	memcpy(sendData.data, stringToSend.c_str(), sendData.dataSize);

	JobDispatcher::GetApi()->Log("Sending %s\n", stringToSend.c_str());
	JobDispatcher::GetApi()->RaiseEvent(FTP_REFRESH_SCREEN_EVENT, new RefreshScreenEventData("Sending " + stringToSend));

	socketApi.sendData(fileDescriptor, sendData);

	delete[] sendData.data;
}

void ClientConnection::HandleUserCommand(const FTPCommand& command) {
	user = configHandler.GetUser(command.args);

	std::string send_string = "330 OK, send password";
	SendResponse(send_string, controlFd);
}

void ClientConnection::HandlePassCommand(const FTPCommand& command) {
	std::string send_string = "530, user/passwd not correct";
	if(nullptr != user) {
		if(command.args == user->passwd) {
			send_string = "230 OK, user " + user->userName + " logged in";
			currDir = user->homeDir;
			loggedIn = true;
		} else {

		}
	}
	SendResponse(send_string, controlFd);
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
	SendResponse(send_string, controlFd);
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

	response += "\r\n";

	std::string send_string = "150 LIST executed ok, data follows";
	SendResponse(send_string, controlFd);

	SendResponse(response, dataFd);

	send_string = "226 LIST data send finished";
	SendResponse(send_string, controlFd);

	socketApi.disconnect(dataFd);
	dataFd = -1;
}

void ClientConnection::HandleQuitCommand() {
	std::string send_string = "221 Bye Bye";
	SendResponse(send_string, controlFd);

	JobDispatcher::GetApi()->UnsubscribeToEvent(controlFd, this);
	JobDispatcher::GetApi()->RaiseEvent(CLIENT_DISCONNECTED_EVENT, new ClientStatusChangeEventData(controlFd));
}

void ClientConnection::HandlePwdCommand() {
	std::string send_string = "257 \"" + currDir + "\"";
	SendResponse(send_string, controlFd);
}
