/*
 * admin_interface.cpp
 *
 *  Created on: Aug 24, 2016
 *      Author: janne
 */

#include "../inc/admin_interface.h"
#include "../inc/thread_fwk/jobdispatcher.h"
#include "../inc/admin_interface_events.h"
#include <iostream>

AdminInterface::AdminInterface() : running(true) {
	JobDispatcher::GetApi()->SubscribeToEvent(FTP_SHUT_DOWN_EVENT_RSP, this);
	JobDispatcher::GetApi()->SubscribeToEvent(FTP_REFRESH_SCREEN_EVENT, this);
	JobDispatcher::GetApi()->SubscribeToEvent(FTP_LIST_CONNECTIONS_EVENT_RSP, this);
	menuStr = "\n--------------------------------------------\n";
	menuStr += "Available commands\n";
	menuStr += "Shutdown FTP server -     exit\n";
	menuStr += "List active connections - list\n\n";
	menuStr += "Command: ";
}

void AdminInterface::Execute() {
	std::string userInput;
	while(running) {
		JobDispatcher::GetApi()->RaiseEvent(FTP_REFRESH_SCREEN_EVENT, nullptr);
		std::cin>>userInput;

		if("exit" == userInput) {
			running = false;
			JobDispatcher::GetApi()->RaiseEvent(FTP_SHUT_DOWN_EVENT, nullptr);
			break;
		} else if("list" == userInput) {
			JobDispatcher::GetApi()->RaiseEvent(FTP_LIST_CONNECTIONS_EVENT, nullptr);
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
	} else if(FTP_REFRESH_SCREEN_EVENT == eventNo) {
		std::lock_guard<std::mutex> printLock(printMutex);

		if(nullptr != dataPtr ){
			const RefreshScreenEventData* screenData = static_cast<const RefreshScreenEventData*>(dataPtr);
			screenBuf.push_back(screenData->str);
		}

		if(screenBuf.size() > 40) {
			screenBuf.erase(screenBuf.begin());
		}

		printf("\033c");
		std::vector<std::string>::iterator screenIter = screenBuf.begin();
		for( ; screenIter != screenBuf.end(); ++screenIter) {
			std::cout<<*screenIter<<std::endl;
		}
		std::cout<<menuStr<<std::endl;
	} else if(FTP_LIST_CONNECTIONS_EVENT_RSP == eventNo) {
		std::lock_guard<std::mutex> printLock(printMutex);

		screenBuf.push_back("Connections:\n");

		const ListConnectionsEventData* connectionsData = static_cast<const ListConnectionsEventData*>(dataPtr);
		screenBuf.push_back(connectionsData->str);

		if(screenBuf.size() > 40) {
			screenBuf.erase(screenBuf.begin());
		}

		printf("\033c");
		std::vector<std::string>::iterator screenIter = screenBuf.begin();
		for( ; screenIter != screenBuf.end(); ++screenIter) {
			std::cout<<*screenIter<<std::endl;
		}
		std::cout<<menuStr<<std::endl;
	}
}
