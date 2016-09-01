/*
 * main.cpp
 *
 *  Created on: Aug 20, 2016
 *      Author: janne
 */

#include "../inc/thread_fwk/jobdispatcher.h"
#include "../inc/server_socket_listener.h"
#include "../inc/admin_interface.h"

int main(void) {
	JobDispatcher::GetApi()->AddExecGroup(DEFAULT_EXEC_GROUP_ID, 0);

	JobDispatcher::GetApi()->Log("FTPServer starting...");

	JobDispatcher::GetApi()->ExecuteJob(new ServerSocketListener());
	JobDispatcher::GetApi()->ExecuteJob(new AdminInterface());

	JobDispatcher::GetApi()->WaitForExecutionFinished();

	JobDispatcher::DropInstance();

	return 0;
}
