/*
 * jobqueueworkercontainer.h
 *
 *  Created on: Sep 1, 2016
 *      Author: janne
 */

#ifndef INC_INTERNAL_JOBQUEUEWORKERCONTAINER_H_
#define INC_INTERNAL_JOBQUEUEWORKERCONTAINER_H_

#include <map>
#include <mutex>
#include "jobqueue.h"
#include "worker.h"
#include "../jobbase.h"

class JobQueueWorkerData
{
public:
	WorkerPtrVectorT workers;
	JobQueue* jobQueuePtr = nullptr;
	uint32_t maxNoOfWorkers = 0;
};

typedef std::map<uint32_t, JobQueueWorkerData> JobQueueMapT;

class JobQueueWorkerContainer
{
public:
	JobQueueWorkerContainer();
	~JobQueueWorkerContainer();
	void AddExecGroup(uint32_t groupId, uint32_t maxNoOfThreads);
	void ScheduleJob(uint32_t groupId, JobBase* jobBase);

protected:

private:
	void CreateExecGroup(uint32_t groupId, uint32_t maxNoOfThreads);

	JobQueueMapT queueMap;
	std::mutex workerCreationMutex;
	std::mutex jobQueueWorkerDataCreationMutex;
};


#endif /* INC_INTERNAL_JOBQUEUEWORKERCONTAINER_H_ */
