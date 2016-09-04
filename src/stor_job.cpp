/*
 * stor_job.cpp
 *
 *  Created on: Sep 4, 2016
 *      Author: janne
 */

#include "../inc/stor_job.h"
#include "../inc/ftp_utils.h"
#include <fstream>

StorJob::StorJob(const std::string& _filePath, int32_t _dataFd, int32_t _controlFd, bool _binaryFlag) :
filePath(_filePath),
dataFd(_dataFd),
controlFd(_controlFd),
binaryFlag(_binaryFlag){

}


void StorJob::Execute() {
	std::ofstream fileStream(filePath);

	std::string send_string =
			"150 STORE ok, send data pretty please";
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
		} while (receiveBuf.dataSize != 0);
	} else {
		unsigned int max_buf = 2048;
		do {
			receiveBuf = socketApi.receiveData(dataFd, max_buf);
			fileStream.write(receiveBuf.data, receiveBuf.dataSize);
			delete[] receiveBuf.data;
		} while (receiveBuf.dataSize == max_buf);
	}

	socketApi.disconnect(dataFd);
	dataFd = -1;

	send_string = "226 STOR data received ok";
	FTPUtils::SendString(send_string, controlFd, socketApi);
}
