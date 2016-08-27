/*
 * server_socket_listener.cpp
 *
 *  Created on: Aug 20, 2016
 *      Author: janne
 */

#include "../inc/server_socket_listener.h"
#include "../inc/admin_interface_events.h"
#include "../inc/thread_fwk/jobdispatcher.h"
#include <string>
#include <sys/time.h>
#include <algorithm>

#define FTP_CLIENT_INACTIVE_CHECK_TIMEOUT 0x30001000

ServerSocketListener::ServerSocketListener() : running(true), noOfCycles(0) {
	JobDispatcher::GetApi()->SubscribeToEvent(CLIENT_DISCONNECTED_EVENT, this);
	JobDispatcher::GetApi()->SubscribeToEvent(CLIENT_INACTIVE_EVENT, this);
	JobDispatcher::GetApi()->SubscribeToEvent(FTP_SHUT_DOWN_EVENT, this);
	JobDispatcher::GetApi()->SubscribeToEvent(FTP_LIST_CONNECTIONS_EVENT, this);
	JobDispatcher::GetApi()->SubscribeToEvent(FTP_CLIENT_INACTIVE_CHECK_TIMEOUT, this);

	JobDispatcher::GetApi()->RaiseEventIn(FTP_CLIENT_INACTIVE_CHECK_TIMEOUT, nullptr, 500);
}

ServerSocketListener::~ServerSocketListener() {
	JobDispatcher::GetApi()->UnsubscribeToEvent(CLIENT_DISCONNECTED_EVENT, this);
	JobDispatcher::GetApi()->UnsubscribeToEvent(CLIENT_INACTIVE_EVENT, this);
	JobDispatcher::GetApi()->UnsubscribeToEvent(FTP_SHUT_DOWN_EVENT, this);
	JobDispatcher::GetApi()->UnsubscribeToEvent(FTP_LIST_CONNECTIONS_EVENT, this);
	JobDispatcher::GetApi()->UnsubscribeToEvent(FTP_CLIENT_INACTIVE_CHECK_TIMEOUT, this);
}

void ServerSocketListener::Execute() {
	int serverFd = socketAPI.getServerSocketFileDescriptor("3370");

	JobDispatcher::GetApi()->Log("FTPServer waiting for connections on port %s",
			"3370");

	while (running) {
		fd_set rfds;
		FD_ZERO(&rfds);
		FD_SET(serverFd, &rfds);

		int32_t maxFd = serverFd;

		ClientConnMapT::iterator connection = clientConnections.begin();
		for (; connection != clientConnections.end(); ++connection) {
			if (false == connection->second->active) {
				FD_SET(connection->second->controlFd, &rfds);
				if(connection->second->controlFd > maxFd) {
					maxFd = connection->second->controlFd;
				}
			}
		}

		timeval timeout;
		timeout.tv_usec = 100000;
		timeout.tv_sec = 0;

		int retval = select(maxFd + 1, &rfds, NULL, NULL, &timeout);

		std::lock_guard<std::mutex> fileDescriptorLock(fileDescriptorMutex);

		if (retval > 0) {
			/* We have something on a controlFd to deal with */
			if(FD_ISSET(serverFd, &rfds)) {
				/* Client connecting to server */
				int clientFd = socketAPI.waitForConnection(serverFd);
				JobDispatcher::GetApi()->Log(
						"FTPServer new connection established. clientFd: %d",
						clientFd);

				ClientConnectionHandler* clientConn =
						new ClientConnectionHandler(clientFd, config);
				clientConnections[clientFd] = clientConn;
			}

			/* Check if we have anything waiting on any alread established connections */
			connection = clientConnections.begin();
			for (; connection != clientConnections.end(); ++connection) {
				if (FD_ISSET(connection->second->controlFd, &rfds)) {
					if (false == connection->second->active) {
						connection->second->active = true;
						JobDispatcher::GetApi()->RaiseEvent(connection->second->controlFd,
								nullptr);
					}
				}
			}
		} else if (retval == 0) {
			//printf("No connections established\n");
		} else {
			perror("select failed\n");
			JobDispatcher::GetApi()->NotifyExecutionFinished();
			break;
		}

		/* Clean up connections that have been disconnected */
		DisconnectInactiveConnections();
	}

	/* Disconnect any lingering still active connections during shut down procedure */
	DisconnectInactiveConnections();

	JobDispatcher::GetApi()->RaiseEvent(FTP_SHUT_DOWN_EVENT_RSP, nullptr);
}

void ServerSocketListener::DisconnectInactiveConnections() {
	ClientConnMapT::iterator connection = clientConnections.begin();
	while (connection != clientConnections.end()) {
		if (true == connection->second->invalid) {
			socketAPI.disconnect(connection->second->controlFd);
			delete connection->second;
			connection = clientConnections.erase(connection);
			continue;
		}
		connection++;
	}
}

void ServerSocketListener::HandleEvent(const uint32_t eventNo,
		const EventDataBase* dataPtr) {
	if (CLIENT_DISCONNECTED_EVENT == eventNo) {

	} else if (CLIENT_INACTIVE_EVENT == eventNo) {

	} else if(FTP_SHUT_DOWN_EVENT == eventNo) {
		/* Mark all connections as inactive to force a disconnect from the other thread */
		std::lock_guard<std::mutex> fileDescriptorLock(fileDescriptorMutex);
		ClientConnMapT::iterator connection = clientConnections.begin();
		for( ; connection != clientConnections.end(); ++connection) {
			connection->second->invalid = true;
		}
		running = false;
	} else if(FTP_LIST_CONNECTIONS_EVENT == eventNo) {
		std::lock_guard<std::mutex> fileDescriptorLock(fileDescriptorMutex);

		std::string response_string = "";

		ClientConnMapT::iterator connection = clientConnections.begin();
		for( ; connection != clientConnections.end(); ++connection) {
			if(connection->second->invalid == false) {
				response_string += "controlFD: " + std::to_string(connection->second->controlFd) + "\n";
			}
		}
		JobDispatcher::GetApi()->RaiseEvent(FTP_LIST_CONNECTIONS_EVENT_RSP, new ListConnectionsEventData(response_string));
	} else if (FTP_CLIENT_INACTIVE_CHECK_TIMEOUT == eventNo) {
		std::lock_guard<std::mutex> fileDescriptorLock(fileDescriptorMutex);
		noOfCycles++;
		if(noOfCycles > 2 * 60 * 5) {
			noOfCycles = 0;
			ClientConnMapT::iterator connection = clientConnections.begin();
			for( ; connection != clientConnections.end(); ++connection) {
				if(connection->second->invalid == false) {
					connection->second->DisconnectIfInactive();
				}
			}
		}
		JobDispatcher::GetApi()->RaiseEventIn(FTP_CLIENT_INACTIVE_CHECK_TIMEOUT, nullptr, 500);
	}
}
