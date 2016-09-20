/*
 * passive_mode_filedesc.cpp
 *
 *  Created on: Sep 19, 2016
 *      Author: janne
 */

#include "../inc/passive_mode_filedesc.h"
#include "../inc/ftp_utils.h"
#include <cstdlib>
#include <string>
#include <algorithm>

PassiveModeFileDesc* PassiveModeFileDesc::instance = nullptr;
std::mutex PassiveModeFileDesc::instanceCreationMutex;


PassiveModeFileDesc::PassiveModeFileDesc() : serverFd(-1), config(ConfigHandler().GetPassiveConfig()) {
	serverFd = socketApi.getServerSocketFileDescriptor(config->portNo);
}

PassiveModeFileDesc* PassiveModeFileDesc::GetApi() {
	if(instance == nullptr) {
		instanceCreationMutex.lock();
		if(instance == nullptr) {
			instance = new PassiveModeFileDesc();
		}
		instanceCreationMutex.unlock();
	}

	return instance;
}

int PassiveModeFileDesc::GetDataFd(int controlFd) {
	std::unique_lock<std::mutex> socketListenerLock(socketListenerMutex);

	std::string send_string = "227 PASV (";

	std::string address = config->address;

	std::replace(address.begin(), address.end(), '.', ',');

	send_string += address + ",";

	int high8 = std::atoi(config->portNo.c_str()) / 256;
	int low8 = std::atoi(config->portNo.c_str()) - (256 * high8);

	send_string += std::to_string(high8) + ",";
	send_string += std::to_string(low8) + ")";

	FTPUtils::SendString(send_string, controlFd, socketApi);

	return socketApi.waitForConnection(serverFd);
}
