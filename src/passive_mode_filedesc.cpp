/*
 * passive_mode_filedesc.cpp
 *
 *  Created on: Sep 19, 2016
 *      Author: janne
 */

#include "../inc/passive_mode_filedesc.h"
#include "../inc/ftp_utils.h"
#include "../inc/thread_fwk/jobdispatcher.h"
#include <cstdlib>
#include <string>
#include <algorithm>
#include <sys/time.h>

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

	if(address == "") {
		address = GetHostIpAddress();
	}

	std::replace(address.begin(), address.end(), '.', ',');

	send_string += address + ",";

	int high8 = std::atoi(config->portNo.c_str()) / 256;
	int low8 = std::atoi(config->portNo.c_str()) - (256 * high8);

	send_string += std::to_string(high8) + ",";
	send_string += std::to_string(low8) + ")";

	FTPUtils::SendString(send_string, controlFd, socketApi);

	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(serverFd, &rfds);

	int32_t maxFd = serverFd;

	timeval timeout;
	if("" != config->timeout) {
		timeout.tv_usec = std::atoi(config->timeout.c_str());
	} else {
		timeout.tv_usec = 250000;
	}
	timeout.tv_sec = 0;

	int retval = select(maxFd + 1, &rfds, NULL, NULL, &timeout);

	if(retval > 0) {
		return socketApi.waitForConnection(serverFd);
	}

	JobDispatcher::GetApi()->Log("PASV: Client did not connect within %d ms", timeout.tv_usec / 1000);

	return -1;
}

std::string PassiveModeFileDesc::GetHostIpAddress() {
	std::string retval;

	std::string resString;
	char buffer[2048];
	std::string command = "ip addr 2>&1";
	FILE* file = popen(command.c_str(), "r");
	while (!feof(file)) {
		if (fgets(buffer, 2048, file) != NULL) {
			resString = std::string(buffer);
			if(resString.find("inet ") != std::string::npos) {
				if(resString.find("127.0.0.1") == std::string::npos) {
					retval = resString;
					retval = retval.substr(retval.find("inet ") + 5);
					retval = retval.substr(0, retval.find("/"));
					break;
				}
			}
		}
	}
	pclose(file);

	std::string result = FTPUtils::ExecProc("ip addr");

    return retval;
}
