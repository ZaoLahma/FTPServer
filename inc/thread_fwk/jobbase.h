/*
 * job.h
 *
 *  Created on: Feb 3, 2016
 *      Author: janne
 */

#ifndef INC_JOBBASE_H_
#define INC_JOBBASE_H_

#include <vector>

class JobDataBase
{
public:
	virtual ~JobDataBase() {}
protected:

private:
};

class JobBase
{
public:
	virtual ~JobBase();

	void SetJobData(JobDataBase* _dataPtr);

	virtual void Execute() = 0;
protected:
	JobDataBase* dataPtr;

private:

};

typedef std::vector<JobBase*> JobBasePtrVectorT;

#endif /* INC_JOBBASE_H_ */
