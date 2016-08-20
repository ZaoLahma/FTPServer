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
#include "client_connection_handler.h"

#include <vector>
#include <mutex>

#define CLIENT_DISCONNECTED_EVENT 0x70007000

class ClientDisconnectedEventData : public EventDataBase {
public:
	ClientDisconnectedEventData(unsigned int fileDescriptor) :
	fileDescriptor(fileDescriptor) {

	}

	EventDataBase* clone() const {
		return new ClientDisconnectedEventData(fileDescriptor);
	}

	unsigned int fileDescriptor;

protected:

private:

};

class ServerSocketListener : public JobBase, public EventListenerBase {
public:
	ServerSocketListener();
	void Execute();
	void HandleEvent(const uint32_t eventNo, const EventDataBase* dataPtr);

protected:

private:
	SocketAPI socketAPI;
	std::vector<int> observedFds;
	std::vector<int> fdsToClose;
	ClientConnMapT clientConnections;
	std::mutex fileDescriptorMutex;
};



#endif /* INC_SERVER_SOCKET_LISTENER_H_ */
