/*
 * client_connection.h
 *
 *  Created on: Sep 1, 2016
 *      Author: janne
 */

#ifndef INC_CLIENT_CONNECTION_H_
#define INC_CLIENT_CONNECTION_H_

#include "config_handler.h"
#include "socket_wrapper/socket_api.h"
#include "thread_fwk/eventlistenerbase.h"

enum FTPCommandEnum {
	NOT_IMPLEMENTED = 0,
	USER,
	PASS,
	PWD,
	PORT,
	LIST,
	QUIT
};

class FTPCommand {
public:
	FTPCommandEnum ftpCommand;
	std::string args;
};

class ClientConnection : public EventListenerBase {
public:
	ClientConnection(int32_t controlFd, ConfigHandler& configHandler);
	void HandleEvent(const uint32_t eventNo, const EventDataBase* dataPtr);

	bool active;
	int32_t controlFd;
protected:


private:
	ClientConnection();
	SocketAPI socketApi;
	int32_t dataFd;
	User* user;
	ConfigHandler& configHandler;
	std::string currDir;
	bool loggedIn;

	FTPCommand GetCommand();
	std::vector<std::string> SplitString(const std::string& string, const std::string& delimiter);
	std::string GetControlChannelData();
	void SendResponse(const std::string& response, int fileDescriptor);

	void HandleUserCommand(const FTPCommand& command);
	void HandlePassCommand(const FTPCommand& command);
	void HandlePortCommand(const FTPCommand& command);
	void HandleListCommand(const FTPCommand& command);
	void HandleQuitCommand();
	void HandlePwdCommand();
};

typedef std::map<int, ClientConnection*> ClientConnMapT;


#endif /* INC_CLIENT_CONNECTION_H_ */
