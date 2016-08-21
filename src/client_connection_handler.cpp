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

ClientConnectionHandler::~ClientConnectionHandler() {
	std::lock_guard<std::mutex> execLock(execMutex);
}

ClientConnectionHandler::ClientConnectionHandler(int fileDescriptor) :
valid(true),
currDir("/Users/janne/GitHub/FTPServer"),
dataFd(-1){
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

	if(stringBuf == "") {
		return;
	}

	printf("stringBuf: %s\n", stringBuf.c_str());

	std::vector<std::string> command = SplitString(stringBuf, ' ');

	if(command.size() == 0) {
		JobDispatcher::GetApi()->Log("ERROR: Couldn't parse command: %s", stringBuf.c_str());
		JobDispatcher::GetApi()->NotifyExecutionFinished();
	}

	if("USER" == command[0]) {
		printf("Sending 230 ok\n");
		std::string send_string = "230 OK, go ahead\r\n";
		SendResponse(send_string, eventNo);
	} else if("PWD" == command[0]) {
		printf("Sending PWD response\n");
		std::string send_string = "257 \"" +  currDir + "\"\r\n";
		SendResponse(send_string, eventNo);
	} else if("PORT" == command[0]) {
		printf("Sending PORT response\n");
		std::vector<std::string> addressInfo = SplitString(command[1], ',');
		std::string ipAddress = addressInfo[0] + "." + addressInfo[1] + "." + addressInfo[2] + "." + addressInfo[3];
		unsigned int portNoInt = atoi(addressInfo[4].c_str()) * 256 + atoi(addressInfo[5].c_str());
		std::string portNo = std::to_string(portNoInt);

		printf("ipAddress: %s, portNo: %s\n", ipAddress.c_str(), portNo.c_str());
		dataFd = socketApi.getClientSocketFileDescriptor(ipAddress, portNo);

		printf("Sending 200 PORT ok\n");
		std::string send_string = "200, PORT command ok\r\n";
		SendResponse(send_string, eventNo);
	} else if("LIST" == command[0]) {
		char buffer[2048];
		std::string response = "150 ";
        std::string ls = "ls -l";
        ls.append(" 2>&1");
        FILE* file = popen(ls.c_str(), "r");

        while (!feof(file)) {
            if (fgets(buffer, 2048, file) != NULL) {
            	response.append(buffer);
            }
        }

        printf("LS: %s\n", response.c_str());
        std::vector<std::string> responseVector = SplitString(response, '\n');
        response = "";
        for(unsigned int i = 0; i < responseVector.size(); ++i) {
        	response += responseVector[i] + "\r\n";
        }

        response += "\r\n";

		printf("Sending 150 LIST ok\n");
		std::string send_string = "150 LIST executed ok, data follows\r\n";
		SendResponse(send_string, eventNo);

		printf("dataFd: %d\n", dataFd);
        SendResponse(response, dataFd);

		printf("Sending 226 LIST ok\n");
		send_string = "226 LIST data send finished\r\n";
		SendResponse(send_string, eventNo);

		socketApi.disconnect(dataFd);
		dataFd = -1;
	} else if("TYPE" == command[0] && "I" == command[1]) {
		printf("Sending 200 ok\n");
		std::string send_string = "200 Binary transfer mode chosen\r\n";
		SendResponse(send_string, eventNo);
	} else if("QUIT" == command[0]) {
		printf("Sending 221 QUIT response\n");
		std::string send_string = "221 Bye Bye\r\n";
		SendResponse(send_string, eventNo);

		JobDispatcher::GetApi()->UnsubscribeToEvent(eventNo, this);
		JobDispatcher::GetApi()->RaiseEvent(CLIENT_DISCONNECTED_EVENT, new ClientDisconnectedEventData(eventNo));
		valid = false;
	} else {
		printf("Sending 500 not implemented response to: %s\n", command[0].c_str());
		std::string send_string = "500 Not implemented\r\n";
		SendResponse(send_string, eventNo);
	}
}

void ClientConnectionHandler::SendResponse(const std::string& response, int fileDescriptor) {
	SocketBuf sendData;
	sendData.dataSize = strlen(response.c_str());
	sendData.data = new char[sendData.dataSize];

	memcpy(sendData.data, response.c_str(), sendData.dataSize);

	socketApi.sendData(fileDescriptor, sendData);

	delete sendData.data;
}

std::vector<std::string> ClientConnectionHandler::SplitString(std::string& str, const char& c) {
	std::vector<std::string> retVal;

	std::string::size_type start_pos = 0;
	std::string::size_type end_pos = str.find(c);

	while (end_pos != std::string::npos) {
		retVal.push_back(str.substr(start_pos, end_pos-start_pos));
		start_pos = ++end_pos;
		end_pos = str.find(c, end_pos);
	}

	if (end_pos == std::string::npos) {
	 retVal.push_back(str.substr(start_pos, str.length()));
	}

	return retVal;
}
