/*
 * eventlistenerbase.h
 *
 *  Created on: Feb 7, 2016
 *      Author: janne
 */

#ifndef INC_EVENTLISTENERBASE_H_
#define INC_EVENTLISTENERBASE_H_

#include <vector>
#include <map>

#include "eventdatabase.h"

class EventListenerBase
{
public:
	virtual ~EventListenerBase();
	virtual void HandleEvent(const uint32_t eventNo, const EventDataBase* dataPtr) = 0;

protected:

private:

};

typedef std::vector<EventListenerBase*> EventListenerBasePtrVector;

typedef std::map<uint32_t, EventListenerBasePtrVector> EventNoToEventListenersMap;

typedef std::map<EventListenerBase*, std::vector<uint32_t>> EventListenerToEventNoMap;


#endif /* INC_EVENTLISTENERBASE_H_ */
