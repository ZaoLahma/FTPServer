/*
 * server_socket_listener.cpp
 *
 *  Created on: Aug 20, 2016
 *      Author: janne
 */

#include "../inc/server_socket_listener.h"
#include "../inc/thread_fwk/jobdispatcher.h"
#include <string>
#include <sys/time.h>
#include <algorithm>

ServerSocketListener::ServerSocketListener() {
	JobDispatcher::GetApi()->SubscribeToEvent(CLIENT_DISCONNECTED_EVENT, this);
	JobDispatcher::GetApi()->SubscribeToEvent(CLIENT_INACTIVE_EVENT, this);
}

void ServerSocketListener::Execute() {
	int serverFd = socketAPI.getServerSocketFileDescriptor("3370");
	observedFds.push_back(serverFd);

	JobDispatcher::GetApi()->Log("FTPServer waiting for connections on port %s",
			"3370");

	while (true) {
		observedFds.clear();
		observedFds.push_back(serverFd);
		ClientConnMapT::iterator connection = clientConnections.begin();
		for (; connection != clientConnections.end(); ++connection) {
			if (false == connection->second->active) {
				//printf("Adding %d to observedFds\n", connection->second->controlFd);
				observedFds.push_back(connection->second->controlFd);
			}
		}

		std::vector<int>::iterator maxIter = std::max(observedFds.begin(),
				observedFds.end() - 1);
		std::vector<int>::iterator fdIter = observedFds.begin();

		fd_set rfds;
		FD_ZERO(&rfds);
		for (; fdIter != observedFds.end(); ++fdIter) {
			FD_SET(*fdIter, &rfds);
		}

		timeval timeout;
		timeout.tv_usec = 100000;
		timeout.tv_sec = 0;
		int retval = select(*maxIter + 1, &rfds, NULL, NULL, &timeout);

		if (retval > 0) {
			std::vector<int> newFds;
			fdIter = observedFds.begin();
			for (; fdIter != observedFds.end(); ++fdIter) {
				if (FD_ISSET(*fdIter, &rfds)) {
					if (*fdIter == serverFd) {
						int clientFd = socketAPI.waitForConnection(serverFd);
						JobDispatcher::GetApi()->Log(
								"FTPServer new connection established. clientFd: %d",
								clientFd);

						ClientConnectionHandler* clientConn =
								new ClientConnectionHandler(clientFd);
						clientConnections[clientFd] = clientConn;
					} else {
						ClientConnMapT::iterator connection =
								clientConnections.find(*fdIter);
						if (false == connection->second->active) {
							connection->second->active = true;
							JobDispatcher::GetApi()->RaiseEvent(*fdIter,
									nullptr);
						}
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

		std::lock_guard<std::mutex> fileDescriptorLock(fileDescriptorMutex);
		connection = clientConnections.begin();
		while (connection != clientConnections.end()) {
			if (true == connection->second->invalid) {
				fdIter = std::find(observedFds.begin(), observedFds.end() - 1,
						connection->second->controlFd);
				if (observedFds.end() != fdIter) {
					observedFds.erase(fdIter);
				}
				socketAPI.disconnect(connection->second->controlFd);

				delete connection->second;
				connection = clientConnections.erase(connection);
				printf("Disconnected a client\n");
				continue;
			}
			connection++;
		}
	}
}

void ServerSocketListener::HandleEvent(const uint32_t eventNo,
		const EventDataBase* dataPtr) {
	std::lock_guard<std::mutex> fileDescriptorLock(fileDescriptorMutex);
	if (CLIENT_DISCONNECTED_EVENT == eventNo) {
		ClientStatusChangeEventData* clientDisconnected =
				(ClientStatusChangeEventData*) (dataPtr);

		ClientConnMapT::iterator connection = clientConnections.find(
				clientDisconnected->fileDescriptor);
		connection->second->invalid = true;
	} else if (CLIENT_INACTIVE_EVENT == eventNo) {
		printf("Received client inactive event\n");
		ClientStatusChangeEventData* clientDisconnected =
				(ClientStatusChangeEventData*) (dataPtr);

		ClientConnMapT::iterator connection = clientConnections.find(
				clientDisconnected->fileDescriptor);
		if (clientConnections.end() != connection) {
			connection->second->active = false;
		}
	}
}
