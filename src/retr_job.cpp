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
	JobDispatcher::GetApi()->Log("filePath: %s", filePath.c_str());

	std::string send_string = "226 RETR data send finished\r\n";
	FTPUtils::SendString(send_string, controlFd, socketApi);

	socketApi.disconnect(dataFd);
	dataFd = -1;
}

void RetrJob::HandleEvent(const uint32_t eventNo, const EventDataBase* dataPtr) {

}
