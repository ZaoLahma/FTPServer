/*
 * admin_interface.h
 *
 *  Created on: Aug 24, 2016
 *      Author: janne
 */

#ifndef INC_ADMIN_INTERFACE_H_
#define INC_ADMIN_INTERFACE_H_

#include "thread_fwk/jobbase.h"
#include "thread_fwk/eventlistenerbase.h"
#include <condition_variable>
#include <mutex>

class AdminInterface : public JobBase, public EventListenerBase {
public:
	AdminInterface();
	void Execute();
	void HandleEvent(const uint32_t eventNo, const EventDataBase* dataPtr);

protected:

private:
	bool running;
	std::mutex shuttingDownMutex;
	std::condition_variable shuttingDownCondition;
};

#endif /* INC_ADMIN_INTERFACE_H_ */
