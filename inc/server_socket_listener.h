/*
 * server_socket_listener.h
 *
 *  Created on: Aug 20, 2016
 *      Author: janne
 */

#ifndef INC_SERVER_SOCKET_LISTENER_H_
#define INC_SERVER_SOCKET_LISTENER_H_

#include "thread_fwk/jobbase.h"
#include "socket_wrapper/socket_api.h"

#include <vector>

class ServerSocketListener : public JobBase {
public:
	void Execute();

protected:

private:
	SocketAPI socketAPI;
	std::vector<int> observedFds;
};



#endif /* INC_SERVER_SOCKET_LISTENER_H_ */
