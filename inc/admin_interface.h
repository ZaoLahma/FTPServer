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
#include <vector>
#include <string>

class AdminInterface : public JobBase, public EventListenerBase {
public:
	AdminInterface();
	~AdminInterface();
	void Execute();
	void HandleEvent(const uint32_t eventNo, const EventDataBase* dataPtr);

protected:

private:
	void Draw(const std::string& str);

	bool running;
	std::mutex shuttingDownMutex;
	std::condition_variable shuttingDownCondition;
	std::mutex printMutex;
	std::vector<std::string> screenBuf;
	std::string menuStr;
	bool loggingEnabled;
};

#endif /* INC_ADMIN_INTERFACE_H_ */
