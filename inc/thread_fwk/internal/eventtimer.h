/*
 * eventtimer.h
 *
 *  Created on: Apr 27, 2016
 *      Author: janne
 */

#ifndef INC_INTERNAL_EVENTTIMER_H_
#define INC_INTERNAL_EVENTTIMER_H_

#include <cinttypes>

#include "../eventdatabase.h"
#include "timerbase.h"

class EventTimer : public TimerBase
{
public:
	EventTimer(const uint32_t eventNo, const EventDataBase* _dataPtr, const uint32_t _ms);
	void TimerFunction();

protected:

private:
	EventTimer();
	const uint32_t eventNo;
	const EventDataBase* eventDataPtr;
};



#endif /* INC_INTERNAL_EVENTTIMER_H_ */
