/*
 * admin_interface.cpp
 *
 *  Created on: Aug 24, 2016
 *      Author: janne
 */

#include "../inc/admin_interface.h"
#include "../inc/thread_fwk/jobdispatcher.h"
#include "../inc/admin_interface_events.h"
#include <string>
#include <iostream>

AdminInterface::AdminInterface() : running(true) {
	JobDispatcher::GetApi()->SubscribeToEvent(FTP_SHUT_DOWN_EVENT_RSP, this);
}

void AdminInterface::Execute() {
	std::string screen;
	std::string userInput;
	while(running) {
		screen = "Commands: \n";
		screen += "Exit\n";

		printf("%s\n", screen.c_str());
		std::cin>>userInput;

		if("Exit" == userInput) {
			running = false;
			JobDispatcher::GetApi()->RaiseEvent(FTP_SHUT_DOWN_EVENT, nullptr);
			break;
		}
	}

	std::unique_lock<std::mutex> shuttingDownLock(shuttingDownMutex);
	shuttingDownCondition.wait(shuttingDownLock);
	JobDispatcher::GetApi()->NotifyExecutionFinished();
}

void AdminInterface::HandleEvent(const uint32_t eventNo, const EventDataBase* dataPtr) {
	if(eventNo == FTP_SHUT_DOWN_EVENT_RSP) {
		std::unique_lock<std::mutex> shuttingDownLock(shuttingDownMutex);
		shuttingDownCondition.notify_one();
	}
}
