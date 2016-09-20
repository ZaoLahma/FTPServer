/*
 * passive_mode_filedesc.cpp
 *
 *  Created on: Sep 19, 2016
 *      Author: janne
 */

#include "../inc/passive_mode_filedesc.h"
#include "../inc/ftp_utils.h"

PassiveModeFileDesc* PassiveModeFileDesc::instance = nullptr;
std::mutex PassiveModeFileDesc::instanceCreationMutex;


PassiveModeFileDesc::PassiveModeFileDesc() : serverFd(-1), portNo(3371) {
	serverFd = socketApi.getServerSocketFileDescriptor(std::to_string(portNo));
}

PassiveModeFileDesc* PassiveModeFileDesc::getApi() {
	if(instance == nullptr) {
		instanceCreationMutex.lock();
		if(instance == nullptr) {
			instance = new PassiveModeFileDesc();
		}
		instanceCreationMutex.unlock();
	}

	return instance;
}

int PassiveModeFileDesc::getDataFd(int controlFd) {
	std::unique_lock<std::mutex> socketListenerLock(socketListenerMutex);

	std::string send_string = "227 PASV (127,0,0,1,13,43)";
	FTPUtils::SendString(send_string, controlFd, socketApi);

	return socketApi.waitForConnection(serverFd);
}
