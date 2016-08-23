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
configFilePath("./config.cfg") {

}

User* ConfigHandler::GetUser(const std::string& userName) {
	printf("GetUser called with userName: %s\n", userName.c_str());
	std::lock_guard<std::mutex> fileLock(fileMutex);

	std::ifstream file(configFilePath.c_str(), std::ifstream::in);
	std::string fileBuf;

	User* userPtr = nullptr;
	std::string state = "NO_STATE";
	printf("Before getline\n");
	while(getline(file, fileBuf)) {
		printf("fileBuf: %s\n", fileBuf.c_str());
    	if(state == "NO_STATE") {
    		if(fileBuf.find("USER") != std::string::npos) {
				state = "USER";
				std::string::size_type pos = fileBuf.find(" ");
				std::string user = fileBuf.substr(pos + 1, fileBuf.length());
				if(user == userName) {
					userPtr = new User();
					userPtr->userName = user;
					userPtr->homeDir = "/";
					userPtr->passwd = "";
					userPtr->rights = READ;
				}
	    	}
	    } else if(state == "USER") {
	    	if(fileBuf.find("HOME_DIR") != std::string::npos) {
				if(nullptr != userPtr) {
					std::string::size_type pos = fileBuf.find(" ");
					std::string dir = fileBuf.substr(pos + 1, fileBuf.length());
					userPtr->homeDir = dir;
				}
	    	} else if(fileBuf.find("RIGHTS") != std::string::npos) {
	    		if(nullptr != userPtr) {
				std::string::size_type pos = fileBuf.find(" ");
				std::string rights = fileBuf.substr(pos + 1, fileBuf.length());
				if("READ" == rights) {
					userPtr->rights = READ;
					} else if("WRITE" == rights) {
						userPtr->rights = WRITE;
					}
	    		}
			} else if(fileBuf.find("PASSWD") != std::string::npos) {
				if(nullptr != userPtr) {
					std::string::size_type pos = fileBuf.find(" ");
					std::string password = fileBuf.substr(pos + 1, fileBuf.length());
					userPtr->passwd = password;
				}
			}
	    }

	    if(fileBuf.find("END_USER") != std::string::npos) {
	    	state = "NO_STATE";
	    	if(nullptr != userPtr) {
	    		break;
	    	}
	    }
	}

	file.close();


	printf("returning: %p\n", userPtr);
	return userPtr;
}
