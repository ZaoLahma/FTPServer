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

ClientConnection::ClientConnection(int32_t _controlFd, ConfigHandler& _configHandler) :
active(false),
controlFd(_controlFd),
dataFd(-1),
user(nullptr),
configHandler(_configHandler){
	JobDispatcher::GetApi()->SubscribeToEvent(controlFd, this);

	std::string initConnStr = "220 OK.";

	SendResponse(initConnStr, controlFd);
}

void ClientConnection::HandleEvent(const uint32_t eventNo, const EventDataBase* dataPtr) {
	active = true;

	FTPCommand command = GetCommand();

	switch(command.ftpCommand) {
	case FTPCommandEnum::USER: {
		user = configHandler.GetUser(command.args);

		std::string send_string = "330 OK, send password";
		SendResponse(send_string, controlFd);
	}
	break;
	case FTPCommandEnum::PASS: {
		std::string send_string = "530, user/passwd not correct";
		if(nullptr != user) {
			if(command.args == user->passwd) {
				send_string = "230 OK, user " + user->userName + " logged in";
			}
		}
		SendResponse(send_string, controlFd);
	}
	break;
	case FTPCommandEnum::QUIT: {
		std::string send_string = "221 Bye Bye";
		SendResponse(send_string, controlFd);

		JobDispatcher::GetApi()->UnsubscribeToEvent(controlFd, this);
		JobDispatcher::GetApi()->RaiseEvent(CLIENT_DISCONNECTED_EVENT,
				new ClientStatusChangeEventData(controlFd));
	}
	default:
		std::string send_string = "500 - Not implemented";
		SendResponse(send_string, controlFd);
		break;
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
