/*
 * jobdispatcher.h
 *
 *  Created on: Feb 3, 2016
 *      Author: janne
 */

#ifndef INC_JOBDISPATCHER_H_
#define INC_JOBDISPATCHER_H_

#include <mutex>
#include <atomic>
#include <map>
#include <cinttypes>
#include <chrono>

#include <vector>
#include <condition_variable>

#include "eventlistenerbase.h"
#include "defaultexecgroups.h"
#include "jobbase.h"

#include "internal/timerstorage.h"
#include "internal/jobqueueworkercontainer.h"


#define TIMEOUT_EVENT_ID 0x0 //Move me to appropriate place

class JobDispatcher
{
public:
	static void DropInstance();

	virtual ~JobDispatcher();

	//Main access point
	static JobDispatcher* GetApi();

	//API
	void Log(const char* formatString, ...);

	void AddExecGroup(uint32_t groupId, uint32_t maxNoOfThreads);

	void ExecuteJob(JobBase* jobPtr);

	void ExecuteJobInGroup(JobBase* jobPtr, uint32_t groupId);

	void ExecuteJobIn(JobBase* jobPtr, const uint32_t ms);

	void ExecuteJobInGroupIn(JobBase* jobPtr, uint32_t groupId, uint32_t ms);

	void SubscribeToEvent(const uint32_t eventNo, EventListenerBase* eventListenerPtr);

	void UnsubscribeToEvent(const uint32_t eventNo, EventListenerBase* eventListenerPtr);

	void RaiseEvent(const uint32_t eventNo, const EventDataBase* eventDataPtr);

	void RaiseEventIn(const uint32_t eventNo, const EventDataBase* eventDataPtr, const uint32_t ms);

	void WaitForExecutionFinished();

	void NotifyExecutionFinished();

protected:

private:
	std::string GetTimeStamp();

	//Private attrbutes
	uint32_t noOfCores;

	std::mutex eventListenersAccessMutex;

	std::mutex executionFinishedNotificationMutex;
	std::condition_variable executionFinishedNotification;

	JobQueueWorkerContainer* jobQueueContainer;

	EventNoToEventListenersMap eventEventListeners;

	TimerStorage timerStorage;

	JobDispatcher();
	static std::mutex instanceCreationMutex;
	static JobDispatcher* instance;
};



#endif /* INC_JOBDISPATCHER_H_ */
