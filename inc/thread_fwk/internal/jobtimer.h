/*
 * jobtimer.h
 *
 *  Created on: Apr 27, 2016
 *      Author: janne
 */

#ifndef INC_INTERNAL_JOBTIMER_H_
#define INC_INTERNAL_JOBTIMER_H_

#include <cinttypes>

#include "../jobbase.h"
#include "timerbase.h"

class JobTimer : public TimerBase
{
public:
	JobTimer(JobBase* _jobPtr, const uint32_t _ms, uint32_t groupId = 0);

	void TimerFunction();

protected:

private:
	JobTimer();
	JobBase* jobPtr;
	uint32_t groupId;
};


#endif /* INC_INTERNAL_JOBTIMER_H_ */
