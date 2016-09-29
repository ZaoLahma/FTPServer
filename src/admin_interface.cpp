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
#include <cstdlib>

AdminInterface::AdminInterface() : running(true), loggingEnabled(false) {
	JobDispatcher::GetApi()->SubscribeToEvent(FTP_SHUT_DOWN_EVENT_RSP, this);
	JobDispatcher::GetApi()->SubscribeToEvent(FTP_REFRESH_SCREEN_EVENT, this);
	JobDispatcher::GetApi()->SubscribeToEvent(FTP_LIST_CONNECTIONS_EVENT_RSP, this);
	menuStr = "\n--------------------------------------------\n";
	menuStr += "Available commands\n";
	menuStr += "List active connections - list\n";
	menuStr += "Toggle screen logging   - log\n";
	menuStr += "Disconnect connectionId - disconnect client\n";
	menuStr += "Shutdown FTP server     - exit\n\n";
	menuStr += "command:\n";
}

AdminInterface::~AdminInterface() {
	JobDispatcher::GetApi()->UnsubscribeToEvent(FTP_SHUT_DOWN_EVENT_RSP, this);
	JobDispatcher::GetApi()->UnsubscribeToEvent(FTP_REFRESH_SCREEN_EVENT, this);
	JobDispatcher::GetApi()->UnsubscribeToEvent(FTP_LIST_CONNECTIONS_EVENT_RSP, this);
}

void AdminInterface::Execute() {
	std::string userInput;
	while(running) {
		Draw("");
		std::cin>>userInput;

		if("exit" == userInput) {
			running = false;
			JobDispatcher::GetApi()->RaiseEvent(FTP_SHUT_DOWN_EVENT, nullptr);
			break;
		} else if("list" == userInput) {
			JobDispatcher::GetApi()->RaiseEvent(FTP_LIST_CONNECTIONS_EVENT, nullptr);
		} else if("log" == userInput) {
			loggingEnabled = !loggingEnabled;
			if(loggingEnabled) {
				Draw("Logging to screen on");
			} else {
				Draw("Logging to screen off");
			}
		} else if(userInput.find("disconnect") != std::string::npos) {
			std::cin>>userInput;
			uint32_t socketFd = std::atoi(userInput.c_str());
			JobDispatcher::GetApi()->RaiseEvent(FTP_DISCONNECT_CLIENT_EVENT, new DisconnectClientEventData(socketFd));
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
		if(!loggingEnabled) {
			return;
		}

		std::string strToDraw = "";
		if(nullptr != dataPtr ){
			const RefreshScreenEventData* screenData = static_cast<const RefreshScreenEventData*>(dataPtr);
			strToDraw = screenData->str;
		}

		Draw(strToDraw);
	} else if(FTP_LIST_CONNECTIONS_EVENT_RSP == eventNo) {
		const ListConnectionsEventData* connectionsData = static_cast<const ListConnectionsEventData*>(dataPtr);
		Draw("Connections:\n" + connectionsData->str);
	}
}

void AdminInterface::Draw(const std::string& str) {
	std::lock_guard<std::mutex> printLock(printMutex);

	if("" != str) {
		screenBuf.push_back(str);
	}

	if(screenBuf.size() > 40) {
		screenBuf.erase(screenBuf.begin());
	}

	printf("\033c");
	std::vector<std::string>::iterator screenIter = screenBuf.begin();
	for( ; screenIter != screenBuf.end(); ++screenIter) {
		std::cout<<*screenIter<<std::endl;
	}
	std::cout<<menuStr;
}
