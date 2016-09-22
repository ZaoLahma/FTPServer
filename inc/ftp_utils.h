/*
 * ftp_utils.h
 *
 *  Created on: Sep 3, 2016
 *      Author: janne
 */

#ifndef INC_FTP_UTILS_H_
#define INC_FTP_UTILS_H_

#include "socket_wrapper/socket_api.h"

class FTPUtils {
public:
	static void SendString(const std::string& string, int32_t fileDescriptor, SocketAPI& socket);
	static std::string ExecProc(const std::string& command);
protected:

private:
};

#endif /* INC_FTP_UTILS_H_ */
