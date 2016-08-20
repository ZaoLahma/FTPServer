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

ClientConnectionHandler::~ClientConnectionHandler() {
	std::lock_guard<std::mutex> execLock(execMutex);
}

ClientConnectionHandler::ClientConnectionHandler(int fileDescriptor) :
valid(true) {
	JobDispatcher::GetApi()->SubscribeToEvent(fileDescriptor, this);

	std::string initConnStr = "220 OK.\r\n";

	SocketBuf initConnBuf;
	initConnBuf.dataSize = strlen(initConnStr.c_str());
	initConnBuf.data = new char[initConnBuf.dataSize];

	memcpy(initConnBuf.data, initConnStr.c_str(), initConnBuf.dataSize);

	socketApi.sendData(fileDescriptor, initConnBuf);

	delete initConnBuf.data;
}

void ClientConnectionHandler::HandleEvent(unsigned int eventNo, const EventDataBase* dataPtr) {
	std::lock_guard<std::mutex> execLock(execMutex);

	if(!valid) {
		return;
	}

	SocketBuf buf;
	buf.data = nullptr;
	buf.dataSize = 0;

	std::string stringBuf;
	buf = socketApi.receiveData(eventNo, 1);
	while(*buf.data != '\r' && *buf.data != '\n' && *buf.data != '\0') {
		stringBuf += std::string(buf.data);
		delete buf.data;

		buf = socketApi.receiveData(eventNo, 1);
	}

	if(nullptr != buf.data) {
		delete buf.data;
	}

	printf("stringBuf: %s\n", stringBuf.c_str());

	if(stringBuf == "") {
		return;
	}

	std::string command;
	std::string subCommand;

	size_t pos = stringBuf.find(' ');

	command = stringBuf.substr(0, pos);
	subCommand = stringBuf.substr(pos + 1, stringBuf.length());

	SocketBuf sendData;
	if("USER" == command) {
		printf("Sending 230 ok\n");
		std::string send_string = "230 OK, go ahead\r\n";
		sendData.dataSize = strlen(send_string.c_str());
		sendData.data = new char[sendData.dataSize];

		memcpy(sendData.data, send_string.c_str(), sendData.dataSize);

		socketApi.sendData(eventNo, sendData);

		delete sendData.data;

	}
	else if("QUIT" == command)
	{
		printf("Sending 221 QUIT response\n");
		std::string send_string = "221 Bye Bye\r\n";
		sendData.dataSize = strlen(send_string.c_str());
		sendData.data = new char[sendData.dataSize];

		memcpy(sendData.data, send_string.c_str(), sendData.dataSize);

		socketApi.sendData(eventNo, sendData);

		delete sendData.data;

		JobDispatcher::GetApi()->UnsubscribeToEvent(eventNo, this);
		JobDispatcher::GetApi()->RaiseEvent(CLIENT_DISCONNECTED_EVENT, new ClientDisconnectedEventData(eventNo));
		valid = false;
	} else {
		printf("Sending 500 not implemented response to: %s\n", command.c_str());
		std::string send_string = "500 UNKNOWN COMMAND\r\n";
		sendData.dataSize = strlen(send_string.c_str());
		sendData.data = new char[sendData.dataSize];

		memcpy(sendData.data, send_string.c_str(), sendData.dataSize);

		socketApi.sendData(eventNo, sendData);

		delete sendData.data;
	}
}
