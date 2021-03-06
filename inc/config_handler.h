/*
 * config_handler.h
 *
 *  Created on: Aug 22, 2016
 *      Author: janne
 */

#ifndef INC_CONFIG_HANDLER_H_
#define INC_CONFIG_HANDLER_H_

#include <string>
#include <mutex>

#define READ  0x1
#define WRITE 0x2

class User {
public:
	std::string userName = "";
	std::string homeDir = "";
	std::string passwd = "";
	uint32_t rights = READ;
};

class PassiveConfig {
public:
	std::string address = "";
	std::string portNo = "";
	std::string timeout = "";
};

class ConfigHandler {
public:
	ConfigHandler();
	User* GetUser(const std::string& userName);
	PassiveConfig* GetPassiveConfig();

protected:

private:
	std::string usersFilePath;
	std::string configFilePath;
	std::mutex fileMutex;
};

#endif /* INC_CONFIG_HANDLER_H_ */
