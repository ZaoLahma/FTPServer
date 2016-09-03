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

class RetrJob : public JobBase {
public:
	RetrJob(const std::string& filePath, int32_t dataFd);
	void Execute();
protected:

private:
	RetrJob();
	std::string filePath;
	int32_t dataFd;
};

#endif /* INC_RETR_JOB_H_ */
