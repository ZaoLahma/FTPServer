/*
 * eventdatabase.h
 *
 *  Created on: Apr 27, 2016
 *      Author: janne
 */

#ifndef INC_EVENTDATABASE_H_
#define INC_EVENTDATABASE_H_


class EventDataBase
{
public:
	virtual ~EventDataBase() {}
	virtual EventDataBase* clone() const = 0;
protected:

public:
};


#endif /* INC_EVENTDATABASE_H_ */
