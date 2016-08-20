/*
 * client_connection_handler.h
 *
 *  Created on: Aug 20, 2016
 *      Author: janne
 */

#ifndef INC_CLIENT_CONNECTION_HANDLER_H_
#define INC_CLIENT_CONNECTION_HANDLER_H_

#include "thread_fwk/eventlistenerbase.h"
#include "socket_wrapper/socket_api.h"
#include <mutex>
#include <map>

class ClientConnectionHandler : public EventListenerBase {
public:
	ClientConnectionHandler(int fileDescriptor);
	~ClientConnectionHandler();
	void HandleEvent(const uint32_t eventNo, const EventDataBase* dataPtr);

protected:

private:
	ClientConnectionHandler();
	SocketAPI socketApi;
	std::mutex execMutex;
	bool valid;
	std::string currDir;
	int dataFd;

	void SendResponse(const std::string& response, int fileDescriptor);
	std::vector<std::string> SplitString(std::string& string, const char& character);
};

typedef std::map<int, ClientConnectionHandler*> ClientConnMapT;

#endif /* INC_CLIENT_CONNECTION_HANDLER_H_ */
