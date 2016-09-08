/*
 * stor_job.cpp
 *
 *  Created on: Sep 4, 2016
 *      Author: janne
 */

#include "../inc/stor_job.h"
#include "../inc/thread_fwk/jobdispatcher.h"
#include "../inc/ftp_utils.h"
#include "../inc/data_handling_events.h"
#include <fstream>

StorJob::StorJob(const std::string& _filePath, int32_t _dataFd, int32_t _controlFd, bool _binaryFlag) :
filePath(_filePath),
dataFd(_dataFd),
controlFd(_controlFd),
binaryFlag(_binaryFlag),
transferActive(true) {
	JobDispatcher::GetApi()->SubscribeToEvent(ABORT_DATA_TRANSFER_EVENT_ID, this);
}

StorJob::~StorJob() {
	JobDispatcher::GetApi()->UnsubscribeToEvent(ABORT_DATA_TRANSFER_EVENT_ID, this);
}

void StorJob::Execute() {
	std::ofstream fileStream(filePath);

	std::string send_string = "150 STORE ok, send data pretty please";
	FTPUtils::SendString(send_string, controlFd, socketApi);

	SocketBuf receiveBuf;
	if (false == binaryFlag) {
		receiveBuf = socketApi.receiveData(dataFd, 1);
		do {
			receiveBuf = socketApi.receiveData(dataFd, 1);
			if (0 != receiveBuf.dataSize && *receiveBuf.data != '\r') {
				fileStream << *receiveBuf.data;
			}
			delete[] receiveBuf.data;
		} while (receiveBuf.dataSize != 0 && transferActive);
	} else {
		unsigned int max_buf = 2048;
		do {
			receiveBuf = socketApi.receiveData(dataFd, max_buf);
			fileStream.write(receiveBuf.data, receiveBuf.dataSize);
			delete[] receiveBuf.data;
		} while (receiveBuf.dataSize == max_buf && transferActive);
	}

	socketApi.disconnect(dataFd);
	dataFd = -1;

	send_string = "226 STOR data received ok";
	FTPUtils::SendString(send_string, controlFd, socketApi);
}

void StorJob::HandleEvent(const uint32_t eventNo, const EventDataBase* dataPtr) {
	if(ABORT_DATA_TRANSFER_EVENT_ID == eventNo) {
		const AbortDataTransferData* eventData = static_cast<const AbortDataTransferData*>(dataPtr);
		if(eventData->dataFd == dataFd) {
			transferActive = false;
			std::string send_string = "426 ABOR OK";
			FTPUtils::SendString(send_string, controlFd, socketApi);
		}
	}
}
