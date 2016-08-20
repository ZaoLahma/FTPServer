/*
 * timereventdata.h
 *
 *  Created on: Apr 27, 2016
 *      Author: janne
 */

#ifndef INC_INTERNAL_TIMEREVENTDATA_H_
#define INC_INTERNAL_TIMEREVENTDATA_H_

#include <cinttypes>
#include "../eventdatabase.h"

class TimerEventData : public EventDataBase
{
public:
	TimerEventData(const uint32_t);

	EventDataBase* clone() const;

	uint32_t GetTimerId() const;

protected:

private:
	const uint32_t timerId;
	TimerEventData();
};



#endif /* INC_INTERNAL_TIMEREVENTDATA_H_ */
