/*
 * admin_interface_events.h
 *
 *  Created on: Aug 24, 2016
 *      Author: janne
 */

#ifndef INC_ADMIN_INTERFACE_EVENTS_H_
#define INC_ADMIN_INTERFACE_EVENTS_H_

#include "thread_fwk/eventdatabase.h"
#include <string>

#define FTP_SHUT_DOWN_EVENT      0x1
#define FTP_SHUT_DOWN_EVENT_RSP  0x2
#define FTP_REFRESH_SCREEN_EVENT 0x3

class RefreshScreenEventData : public EventDataBase {
public:
	RefreshScreenEventData(const std::string& _str): str(_str) {
		std::string::size_type pos = str.find('\r');

		if(std::string::npos != pos) {
			str = str.substr(0, pos);
		}
	}
	std::string str;

	EventDataBase* clone() const {
		return new RefreshScreenEventData(str);
	}
protected:

private:
	RefreshScreenEventData();
};

#endif /* INC_ADMIN_INTERFACE_EVENTS_H_ */
