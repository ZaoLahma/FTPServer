/*
 * server_socket_listener.h
 *
 *  Created on: Aug 20, 2016
 *      Author: janne
 */

#ifndef INC_SERVER_SOCKET_LISTENER_H_
#define INC_SERVER_SOCKET_LISTENER_H_

#include "thread_fwk/jobbase.h"
#include "thread_fwk/eventlistenerbase.h"
#include "socket_wrapper/socket_api.h"
#include "client_connection.h"

#include <vector>
#include <mutex>

#define CLIENT_DISCONNECTED_EVENT 0x70007000
#define CLIENT_INACTIVE_EVENT     0x70007001

class ClientStatusChangeEventData : public EventDataBase {
public:
	ClientStatusChangeEventData(unsigned int fileDescriptor) :
	fileDescriptor(fileDescriptor) {

	}

	EventDataBase* clone() const {
		return new ClientStatusChangeEventData(fileDescriptor);
	}

	unsigned int fileDescriptor;

protected:

private:

};

class ServerSocketListener : public JobBase, public EventListenerBase {
public:
	ServerSocketListener();
	~ServerSocketListener();
	void Execute();
	void HandleEvent(const uint32_t eventNo, const EventDataBase* dataPtr);

protected:

private:
	SocketAPI socketAPI;
	std::vector<int> observedFds;
	ClientConnMapT clientConnections;
	std::mutex fileDescriptorMutex;
	ConfigHandler config;
	bool running;
	void DisconnectInactiveConnections();
};



#endif /* INC_SERVER_SOCKET_LISTENER_H_ */
