/*
 * worker.h
 *
 *  Created on: Apr 27, 2016
 *      Author: janne
 */

#ifndef INC_INTERNAL_WORKER_H_
#define INC_INTERNAL_WORKER_H_

#include <atomic>
#include "jobqueue.h"
#include "threadobject.h"

class Worker : public ThreadObject
{
public:
	Worker(JobQueue* _queuePtr);

	void Notify();

	bool IsIdling();

	uint32_t GetNoOfJobsExecuted();

	void Stop();

protected:

private:
	Worker();
	uint32_t noOfJobsExecuted;
	JobQueue* queuePtr;
	std::mutex executionNotificationMutex;
	std::unique_lock<std::mutex> executionLock;
	std::condition_variable executionNotification;

	std::atomic<bool> isIdling;

	void run();
};



#endif /* INC_INTERNAL_WORKER_H_ */
