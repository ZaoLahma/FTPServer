/*
 * data_handling_events.h
 *
 *  Created on: Sep 7, 2016
 *      Author: janne
 */

#ifndef INC_DATA_HANDLING_EVENTS_H_
#define INC_DATA_HANDLING_EVENTS_H_

#include "thread_fwk/eventdatabase.h"

#define ABORT_DATA_TRANSFER_EVENT_ID 0x80009000

class AbortDataTransferData : public EventDataBase {
public:
	AbortDataTransferData(int32_t _dataFd) : dataFd(_dataFd) {

	}

	EventDataBase* clone() const {
		return new AbortDataTransferData(dataFd);
	}

	int32_t dataFd;

protected:

private:
	AbortDataTransferData();
};

#endif /* INC_DATA_HANDLING_EVENTS_H_ */
