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
	RETR,
	TYPE,
	STOR,
	DELE,
	ABOR,
	SYST,
	PASV,
	CWD,
	MKD,
	RMD,
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
	~ClientConnection();
	void HandleEvent(const uint32_t eventNo, const EventDataBase* dataPtr);

	void CheckIfInactive();
	void ForceDisconnect();

	bool IsDisconnected();

	bool active;
	int32_t controlFd;
protected:


private:
	ClientConnection();
	SocketAPI socketApi;
	std::string ftpRootDir;
	int32_t dataFd;
	User* user;
	ConfigHandler& configHandler;
	std::string currDir;
	bool loggedIn;
	bool binaryFlag;
	bool inactiveTooLong;
	bool disconnected;
	uint32_t noOfCycles;

	FTPCommand GetCommand();
	std::vector<std::string> SplitString(const std::string& string, const std::string& delimiter);
	std::string GetControlChannelData();

	void HandleUserCommand(const FTPCommand& command);
	void HandlePassCommand(const FTPCommand& command);
	void HandlePortCommand(const FTPCommand& command);
	void HandleListCommand(const FTPCommand& command);
	void HandleRetrCommand(const FTPCommand& command);
	void HandleTypeCommand(const FTPCommand& command);
	void HandleStorCommand(const FTPCommand& command);
	void HandleDeleCommand(const FTPCommand& command);
	void HandleCwdCommand(const FTPCommand& command);
	void HandleMkdCommand(const FTPCommand& command);
	void HandleRmdCommand(const FTPCommand& command);
	void HandleQuitCommand();
	void HandlePasvCommand();
	void HandleAborCommand();
	void HandleSystCommand();
	void HandlePwdCommand();
};

typedef std::map<int, ClientConnection*> ClientConnMapT;


#endif /* INC_CLIENT_CONNECTION_H_ */
