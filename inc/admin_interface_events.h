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

#define FTP_SHUT_DOWN_EVENT            0x50001000
#define FTP_SHUT_DOWN_EVENT_RSP        0x50001001
#define FTP_REFRESH_SCREEN_EVENT       0x50001002
#define FTP_LIST_CONNECTIONS_EVENT     0x50001003
#define FTP_LIST_CONNECTIONS_EVENT_RSP 0x50001004
#define FTP_DISCONNECT_CLIENT_EVENT    0x50001005

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

class ListConnectionsEventData : public EventDataBase {
public:
	ListConnectionsEventData(const std::string& _str): str(_str) {
		std::string::size_type pos = str.find('\r');

		if(std::string::npos != pos) {
			str = str.substr(0, pos);
		}
	}
	std::string str;

	EventDataBase* clone() const {
		return new ListConnectionsEventData(str);
	}
protected:

private:
	ListConnectionsEventData();
};

class DisconnectClientEventData : public EventDataBase {
public:
	DisconnectClientEventData(uint32_t clientFd_): clientFd(clientFd_) {

	}

	uint32_t clientFd;

	EventDataBase* clone() const {
		return new DisconnectClientEventData(clientFd);
	}
protected:

private:
	DisconnectClientEventData();
};

#endif /* INC_ADMIN_INTERFACE_EVENTS_H_ */
