/*
 * retr_job.cpp
 *
 *  Created on: Sep 3, 2016
 *      Author: janne
 */
#include "../inc/retr_job.h"
#include <fstream>
#include "../inc/thread_fwk/jobdispatcher.h"
#include "../inc/ftp_utils.h"

RetrJob::RetrJob(const std::string& _filePath, int32_t _dataFd, int32_t _controlFd, bool _binaryFlag) :
filePath(_filePath),
dataFd(_dataFd),
controlFd(_controlFd),
binaryFlag(_binaryFlag){

}

RetrJob::~RetrJob() {

}


void RetrJob::Execute() {
	std::ifstream fileStream(filePath.c_str(), std::ifstream::binary);
	fileStream.seekg(0, fileStream.end);
	int length = fileStream.tellg();
	fileStream.seekg(0, fileStream.beg);

	std::string send_string = "150 RETR executed ok, data follows\r\n";
	FTPUtils::SendString(send_string, controlFd, socketApi);

	JobDispatcher::GetApi()->Log("filePath: %s, size: %d", filePath.c_str(), length);

	SocketBuf sendBuf;
	sendBuf.dataSize = 1;
	sendBuf.data = new char[1];
	while (length > 0) {
		char c = fileStream.get();
		if (c == '\n') {
			*sendBuf.data = '\r';
			socketApi.sendData(dataFd, sendBuf);
		}
		*sendBuf.data = c;
		socketApi.sendData(dataFd, sendBuf);
		length -= 1;
	}

	delete sendBuf.data;

	send_string = "226 RETR data send finished\r\n";
	FTPUtils::SendString(send_string, controlFd, socketApi);

	socketApi.disconnect(dataFd);
	dataFd = -1;
}

void RetrJob::HandleEvent(const uint32_t eventNo, const EventDataBase* dataPtr) {

}
