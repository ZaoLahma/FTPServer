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

	std::string send_string = "150 RETR executed ok, data follows";
	FTPUtils::SendString(send_string, controlFd, socketApi);

	JobDispatcher::GetApi()->Log("filePath: %s, size: %d", filePath.c_str(), length);

	SocketBuf sendBuf;
	if(false == binaryFlag) {
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
	} else {
		unsigned int max_buf = 2048;
		sendBuf.data = new char[max_buf];
		while (length > 0) {
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
		}
	}

	delete sendBuf.data;

	socketApi.disconnect(dataFd);
	dataFd = -1;

	send_string = "226 RETR data send finished";
	FTPUtils::SendString(send_string, controlFd, socketApi);
}

void RetrJob::HandleEvent(const uint32_t eventNo, const EventDataBase* dataPtr) {

}
