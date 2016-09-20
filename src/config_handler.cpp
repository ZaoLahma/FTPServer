/*
 * config_handler.cpp
 *
 *  Created on: Aug 22, 2016
 *      Author: janne
 */

#include "../inc/config_handler.h"
#include <iostream>
#include <fstream>

ConfigHandler::ConfigHandler() :
		usersFilePath("./users.cfg"),
		configFilePath("./config.cfg") {
}

User* ConfigHandler::GetUser(const std::string& userName) {
	std::lock_guard<std::mutex> fileLock(fileMutex);

	std::ifstream file(usersFilePath.c_str(), std::ifstream::in);
	std::string fileBuf;

	User* userPtr = nullptr;
	std::string state = "NO_STATE";
	while (getline(file, fileBuf)) {
		if (state == "NO_STATE") {
			if (fileBuf.find("USER") != std::string::npos) {
				state = "USER";
				std::string::size_type pos = fileBuf.find(" ");
				std::string user = fileBuf.substr(pos + 1, fileBuf.length());
				if (user == userName) {
					userPtr = new User();
					userPtr->userName = user;
					userPtr->homeDir = "/";
					userPtr->passwd = "";
					userPtr->rights = READ;
				}
			}
		} else if (state == "USER") {
			if (fileBuf.find("HOME_DIR") != std::string::npos) {
				if (nullptr != userPtr) {
					std::string::size_type pos = fileBuf.find(" ");
					std::string dir = fileBuf.substr(pos + 1, fileBuf.length());
					userPtr->homeDir = dir;
				}
			} else if (fileBuf.find("RIGHTS") != std::string::npos) {
				if (nullptr != userPtr) {
					std::string::size_type pos = fileBuf.find(" ");
					std::string rights = fileBuf.substr(pos + 1,
							fileBuf.length());
					if ("READ" == rights) {
						userPtr->rights = READ;
					} else if ("WRITE" == rights) {
						userPtr->rights = WRITE;
					}
				}
			} else if (fileBuf.find("PASSWD") != std::string::npos) {
				if (nullptr != userPtr) {
					std::string::size_type pos = fileBuf.find(" ");
					std::string password = fileBuf.substr(pos + 1,
							fileBuf.length());
					userPtr->passwd = password;
				}
			}
		}

		if (fileBuf.find("END_USER") != std::string::npos) {
			state = "NO_STATE";
			if (nullptr != userPtr) {
				break;
			}
		}
	}

	file.close();
	return userPtr;
}

PassiveConfig* ConfigHandler::GetPassiveConfig() {
	std::lock_guard<std::mutex> fileLock(fileMutex);

	std::ifstream file(configFilePath.c_str(), std::ifstream::in);
	std::string fileBuf;

	PassiveConfig* configPtr = nullptr;
	std::string state = "NO_STATE";

	while (getline(file, fileBuf)) {
		if("NO_STATE" == state) {
			if(fileBuf.find("PASSIVE") != std::string::npos) {
				state = "PASSIVE";
				configPtr = new PassiveConfig();
			}
		} else if("PASSIVE" == state) {
			if(fileBuf.find("PORT") != std::string::npos) {
				std::string::size_type pos = fileBuf.find(" ");
				std::string portNo = fileBuf.substr(pos + 1, fileBuf.length());
				configPtr->portNo = portNo;
			} else if(fileBuf.find("ADDR") != std::string::npos) {
				std::string::size_type pos = fileBuf.find(" ");
				std::string addr = fileBuf.substr(pos + 1, fileBuf.length());
				configPtr->address = addr;
			} else if(fileBuf.find("CONNECT_TIMEOUT") != std::string::npos) {
				std::string::size_type pos = fileBuf.find(" ");
				std::string timeout = fileBuf.substr(pos + 1, fileBuf.length());
				configPtr->timeout = timeout;
			} else if(fileBuf.find("END_PASSIVE") != std::string::npos) {
				state = "NO_STATE";
			}
		}
	}

	file.close();
	return configPtr;
}
