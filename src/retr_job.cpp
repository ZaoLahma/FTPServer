/*
 * retr_job.cpp
 *
 *  Created on: Sep 3, 2016
 *      Author: janne
 */
#include "../inc/retr_job.h"
#include <fstream>

RetrJob::RetrJob(const std::string& _filePath, int32_t _dataFd) :
filePath(_filePath),
dataFd(_dataFd) {

}


void RetrJob::Execute() {
	std::ifstream fileStream(filePath.c_str(), std::ifstream::binary);
	fileStream.seekg(0, fileStream.end);
	int32_t length = fileStream.tellg();
	fileStream.seekg(0, fileStream.beg);
}
