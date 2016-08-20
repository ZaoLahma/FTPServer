/*
 * timerbase.h
 *
 *  Created on: Apr 27, 2016
 *      Author: janne
 */

#ifndef INC_INTERNAL_TIMERBASE_H_
#define INC_INTERNAL_TIMERBASE_H_

#include <cinttypes>
#include <map>

#include "../jobbase.h"
#include "threadobject.h"
#include "timereventdata.h"

class TimerBase : public ThreadObject
{
public:
	TimerBase(const uint32_t _ms);
	virtual ~TimerBase() {}

	void SetTimerId(const uint32_t _timerId);

	void run();

	virtual void TimerFunction() = 0;

protected:
	const uint32_t ms;
	uint32_t timerId;

private:
	TimerBase();
};

typedef std::map<uint32_t, TimerBase*> TimerBaseMap;



#endif /* INC_INTERNAL_TIMERBASE_H_ */
