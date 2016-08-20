/*
 * timerstorage.h
 *
 *  Created on: Apr 27, 2016
 *      Author: janne
 */

#ifndef INC_INTERNAL_TIMERSTORAGE_H_
#define INC_INTERNAL_TIMERSTORAGE_H_

#include "timerbase.h"
#include <mutex>
#include "../eventdatabase.h"
#include "../eventlistenerbase.h"

class TimerStorage : public EventListenerBase
{
public:
	TimerStorage();
	~TimerStorage();
	void StoreTimer(TimerBase* _timer);
	void HandleEvent(const uint32_t _eventNo, const EventDataBase* _dataPtr);

protected:

private:
	bool subscribedToEvent;
	std::mutex subscribeMutex;
	TimerBaseMap timers;
	std::mutex timerMutex;
};



#endif /* INC_INTERNAL_TIMERSTORAGE_H_ */
