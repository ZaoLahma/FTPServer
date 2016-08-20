/*
 * eventjob.h
 *
 *  Created on: Apr 27, 2016
 *      Author: janne
 */

#ifndef INC_INTERNAL_EVENTJOB_H_
#define INC_INTERNAL_EVENTJOB_H_

#include "../eventdatabase.h"
#include "../eventlistenerbase.h"
#include "../jobbase.h"

class EventJob : public JobBase
{
public:
	EventJob(EventListenerBase* _eventListenerPtr,
			 const uint32_t _eventNo,
			 const EventDataBase* _eventDataPtr);

	void Execute();
protected:
private:
	EventJob();

	EventListenerBase* eventListenerPtr;

	const uint32_t eventNo;

	EventDataBase* eventDataPtr;
};



#endif /* INC_INTERNAL_EVENTJOB_H_ */
