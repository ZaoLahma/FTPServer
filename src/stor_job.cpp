/*
 * stor_job.cpp
 *
 *  Created on: Sep 4, 2016
 *      Author: janne
 */

#include "../inc/stor_job.h"

StorJob::StorJob(const std::string& _filePath, int32_t _dataFd, int32_t _controlFd, bool _binaryFlag) :
filePath(_filePath),
dataFd(_dataFd),
controlFd(_controlFd),
binaryFlag(_binaryFlag){

}


void StorJob::Execute() {

}
