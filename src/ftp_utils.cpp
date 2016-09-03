/*
 * ftp_utils.cpp
 *
 *  Created on: Sep 3, 2016
 *      Author: janne
 */

#include "../inc/ftp_utils.h"
#include "../inc/admin_interface_events.h"
#include "../inc/thread_fwk/jobdispatcher.h"

void FTPUtils::SendString(const std::string& string, int32_t fileDescriptor, SocketAPI& socketApi) {
	std::string stringToSend = string;
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
