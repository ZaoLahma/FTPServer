/*
 * retr_job.h
 *
 *  Created on: Sep 3, 2016
 *      Author: janne
 */

#ifndef INC_RETR_JOB_H_
#define INC_RETR_JOB_H_

#include <string>
#include "thread_fwk/jobbase.h"
#include "thread_fwk/eventlistenerbase.h"
#include "socket_wrapper/socket_api.h"

#define DATA_TRANSFER_COMPLETE_EVENT_NO 0x60006000

class RetrJob : public JobBase, public EventListenerBase {
public:
	RetrJob(const std::string& filePath, int32_t dataFd, int32_t controlFd, bool binaryFlag);
	~RetrJob();
	void Execute();
	void HandleEvent(const uint32_t eventNo, const EventDataBase* dataPtr);

protected:

private:
	RetrJob();
	SocketAPI socketApi;
	std::string filePath;
	int32_t dataFd;
	int32_t controlFd;
	bool binaryFlag;
};

#endif /* INC_RETR_JOB_H_ */
