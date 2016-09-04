/*
 * stor_job.h
 *
 *  Created on: Sep 4, 2016
 *      Author: janne
 */

#ifndef INC_STOR_JOB_H_
#define INC_STOR_JOB_H_

#include "thread_fwk/jobbase.h"
#include "socket_wrapper/socket_api.h"
#include <string>

class StorJob : public JobBase {
public:
	StorJob(const std::string& filePath, int32_t dataFd, int32_t controlFd, bool binaryFlag);

	void Execute();

protected:

private:
	StorJob();
	SocketAPI socketApi;
	std::string filePath;
	int32_t dataFd;
	int32_t controlFd;
	bool binaryFlag;
};

#endif /* INC_STOR_JOB_H_ */
