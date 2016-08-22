/*
 * config_handler.h
 *
 *  Created on: Aug 22, 2016
 *      Author: janne
 */

#ifndef INC_CONFIG_HANDLER_H_
#define INC_CONFIG_HANDLER_H_

#include <string>
#include <thread>

#define READ  0x1
#define WRITE 0x2

struct User {
	std::string userName;
	std::string homeDir;
	std::string passwd;
	unsigned int rights;
};

class ConfigHandler {
public:
	ConfigHandler();
	User* GetUser(const std::string& userName);

protected:

private:
	std::string configFilePath;
	std::mutex fileMutex;
};

#endif /* INC_CONFIG_HANDLER_H_ */
