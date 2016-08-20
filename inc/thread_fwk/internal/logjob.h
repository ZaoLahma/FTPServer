/*
 * logjob.h
 *
 *  Created on: Apr 27, 2016
 *      Author: janne
 */

#ifndef INC_INTERNAL_LOGJOB_H_
#define INC_INTERNAL_LOGJOB_H_

#include <mutex>
#include <iostream>
#include <fstream>

#include "../jobbase.h"

class LogJob : public JobBase
{
public:
	LogJob(const std::string& _stringToPrint);

	void Execute();
protected:
private:
	LogJob();

	std::string stringToPrint;
	std::ofstream fileStream;
	static std::mutex fileAccessMutex;

};


#endif /* INC_INTERNAL_LOGJOB_H_ */
