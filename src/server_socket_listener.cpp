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

ServerSocketListener::ServerSocketListener() {
	JobDispatcher::GetApi()->SubscribeToEvent(CLIENT_DISCONNECTED_EVENT, this);
}

void ServerSocketListener::Execute() {
	int serverFd = socketAPI.getServerSocketFileDescriptor("3370");
	observedFds.push_back(serverFd);

	JobDispatcher::GetApi()->Log("FTPServer waiting for connections on port %s", "3370");

	while(true) {

		std::vector<int>::iterator maxIter = std::max(observedFds.begin(), observedFds.end() - 1);
		std::vector<int>::iterator fdIter = observedFds.begin();

		fd_set rfds;
		FD_ZERO(&rfds);
		for( ; fdIter != observedFds.end(); ++fdIter) {
			FD_SET(*fdIter, &rfds);
		}

		timeval timeout;
		timeout.tv_usec = 0;
		timeout.tv_sec = 5;
		int retval = select(*maxIter + 1, &rfds, NULL, NULL, &timeout);

		if(retval > 0)
		{
			std::vector<int> newFds;
			fdIter = observedFds.begin();
			for( ; fdIter != observedFds.end(); ++fdIter) {
				if(FD_ISSET(*fdIter, &rfds)) {
					if(*fdIter == serverFd) {
						int clientFd = socketAPI.waitForConnection(serverFd);
						JobDispatcher::GetApi()->Log("FTPServer new connection established");

						ClientConnectionHandler* clientConn = new ClientConnectionHandler(clientFd);
						clientConnections[clientFd] = clientConn;
						newFds.push_back(clientFd);
					}
					else
					{
						JobDispatcher::GetApi()->RaiseEvent(*fdIter, nullptr);
					}
				}
			}
			fdIter = newFds.begin();
			for(; fdIter != newFds.end(); ++fdIter) {
				observedFds.push_back(*fdIter);
			}
		}
		else if(retval == 0) {
			//printf("No connections established\n");
		}
		else {
			perror("select failed\n");
			JobDispatcher::GetApi()->NotifyExecutionFinished();
			break;
		}

		std::lock_guard<std::mutex> fileDescriptorLock(fileDescriptorMutex);
		std::vector<int>::iterator fdsToCloseIter = fdsToClose.begin();
		for(; fdsToCloseIter != fdsToClose.end(); ++fdsToCloseIter) {
			fdIter = std::find(observedFds.begin(), observedFds.end() - 1, *fdsToCloseIter);
			if(observedFds.end() != fdIter) {
				observedFds.erase(fdIter);
			}
			socketAPI.disconnect(*fdsToCloseIter);

			ClientConnMapT::iterator clientConn = clientConnections.find(*fdsToCloseIter);

			if(clientConn != clientConnections.end()) {
				delete clientConn->second;
				clientConnections.erase(clientConn);
			}
			printf("Disconnected a client\n");
		}
		fdsToClose.clear();
	}
}

void ServerSocketListener::HandleEvent(const uint32_t eventNo, const EventDataBase* dataPtr) {
	std::lock_guard<std::mutex> fileDescriptorLock(fileDescriptorMutex);
	if(CLIENT_DISCONNECTED_EVENT == eventNo){
		ClientDisconnectedEventData* clientDisconnected = (ClientDisconnectedEventData*)(dataPtr);

		fdsToClose.push_back(clientDisconnected->fileDescriptor);
	}
}
