/*
 * passive_mode_filedesc.cpp
 *
 *  Created on: Sep 19, 2016
 *      Author: janne
 */

#include "../inc/passive_mode_filedesc.h"

PassiveModeFileDesc* PassiveModeFileDesc::instance = nullptr;
std::mutex PassiveModeFileDesc::instanceCreationMutex;


PassiveModeFileDesc::PassiveModeFileDesc() : serverFd(-1), portNo(3370) {
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
