/*
 * SocketAPI.h
 *
 *  Created on: Aug 17, 2016
 *      Author: janne
 */

#ifndef INC_SOCKETAPI_H_
#define INC_SOCKETAPI_H_

#include <string>

struct SocketBuf
{
	unsigned int dataSize;
	char* data;
};

class SocketAPI
{
public:
	int getClientSocketFileDescriptor(const std::string& address, const std::string& portNo);

	int getServerSocketFileDescriptor(const std::string& portNo);

	int waitForConnection(int socketFileDescriptor);

	SocketBuf receiveData(int fileDescriptor, int noOfBytes);

	int sendData(int fileDescriptor, const SocketBuf& socketBuf);

	void disconnect(int fileDescriptor);

protected:

private:
};


#endif /* INC_SOCKETAPI_H_ */
