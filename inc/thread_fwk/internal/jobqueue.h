/*
 * jobqueue.h
 *
 *  Created on: Apr 27, 2016
 *      Author: janne
 */

#ifndef INC_INTERNAL_JOBQUEUE_H_
#define INC_INTERNAL_JOBQUEUE_H_

#include <mutex>
#include "../jobbase.h"

class JobQueue
{
public:
	JobQueue();

	~JobQueue();

	void QueueJob(JobBase* jobPtr);
	JobBase* GetNextJob();
protected:
private:
	std::mutex queueAccessMutex;
	std::mutex getJobMutex;

	JobBasePtrVectorT queue_1;
	JobBasePtrVectorT queue_2;

	JobBasePtrVectorT* currentQueue;
	JobBasePtrVectorT* queueToExecute;

	JobBasePtrVectorT::iterator currentElement;

};


#endif /* INC_INTERNAL_JOBQUEUE_H_ */
