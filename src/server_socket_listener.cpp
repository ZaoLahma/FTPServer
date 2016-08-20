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
			fdIter = observedFds.begin();
			for( ; fdIter != observedFds.end(); ++fdIter) {
				if(FD_ISSET(*fdIter, &rfds)) {
					if(*fdIter == serverFd) {
						int clientFd = socketAPI.waitForConnection(serverFd);
						JobDispatcher::GetApi()->Log("FTPServer new connection established");

						std::string initConnStr = "220 OK.\r\n";

						SocketBuf initConnBuf;
						initConnBuf.dataSize = strlen(initConnStr.c_str());
						initConnBuf.data = new char[initConnBuf.dataSize];

						memcpy(initConnBuf.data, initConnStr.c_str(), initConnBuf.dataSize);

						socketAPI.sendData(clientFd, initConnBuf);

						delete initConnBuf.data;

						observedFds.push_back(clientFd);
					}
					else
					{
						//Start a new job handling the already established connection
					}
				}
			}
		}
		else if(retval == 0) {
			printf("No connections established\n");
		}
		else {
			perror("select failed\n");
			JobDispatcher::GetApi()->NotifyExecutionFinished();
			break;
		}
	}
}
