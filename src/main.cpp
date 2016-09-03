/*
 * main.cpp
 *
 *  Created on: Aug 20, 2016
 *      Author: janne
 */

#include "../inc/thread_fwk/jobdispatcher.h"
#include "../inc/server_socket_listener.h"
#include "../inc/admin_interface.h"
#include "../inc/ftp_thread_model.h"

int main(void) {
	JobDispatcher::GetApi()->AddExecGroup(DEFAULT_EXEC_GROUP_ID, 0);

	JobDispatcher::GetApi()->AddExecGroup(SOCKET_LISTENER_THREAD_GROUP_ID,
										  SOCKET_LISTENER_THREAD_GROUP_MAX_NO_OF_THREADS);

	JobDispatcher::GetApi()->AddExecGroup(DATA_CHANNEL_THREAD_GROUP_ID,
										  DATA_CHANNEL_THREAD_GROUP_MAX_NO_OF_THREADS);

	JobDispatcher::GetApi()->AddExecGroup(ADMIN_THREAD_GROUP_ID,
										  ADMIN_THREAD_GROUP_MAX_NO_OF_THREADS);

	JobDispatcher::GetApi()->Log("FTPServer starting...");

	JobDispatcher::GetApi()->ExecuteJobInGroup(new ServerSocketListener(), SOCKET_LISTENER_THREAD_GROUP_ID);
	JobDispatcher::GetApi()->ExecuteJobInGroup(new AdminInterface(), ADMIN_THREAD_GROUP_ID);

	JobDispatcher::GetApi()->WaitForExecutionFinished();

	JobDispatcher::DropInstance();

	return 0;
}
