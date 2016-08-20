/*
 * main.cpp
 *
 *  Created on: Aug 20, 2016
 *      Author: janne
 */

#include "../inc/thread_fwk/jobdispatcher.h"
#include "../inc/server_socket_listener.h"

int main(void) {
	JobDispatcher::GetApi()->Log("FTPServer starting...");

	JobDispatcher::GetApi()->ExecuteJob(new ServerSocketListener());

	JobDispatcher::GetApi()->WaitForExecutionFinished();
	return 0;
}
