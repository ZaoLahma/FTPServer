/*
 * passive_mode_filedesc.h
 *
 *  Created on: Sep 19, 2016
 *      Author: janne
 */

#ifndef INC_PASSIVE_MODE_FILEDESC_H_
#define INC_PASSIVE_MODE_FILEDESC_H_

#include <thread>
#include <mutex>
#include "socket_wrapper/socket_api.h"

/**
 * This class shall...
 * Read passive mode configuration parameters from configuration file
 * Provide a thread safe interface for setting up the passive mode data connection
 */

class PassiveModeFileDesc {
public:
	static PassiveModeFileDesc* getApi();

	int getDataFd(int controlFd);

protected:

private:
	PassiveModeFileDesc();
	int serverFd;
	int portNo;
	SocketAPI socketApi;
	static PassiveModeFileDesc* instance;
	static std::mutex instanceCreationMutex;
	std::mutex socketListenerMutex;
};



#endif /* INC_PASSIVE_MODE_FILEDESC_H_ */
