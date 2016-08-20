/*
 * uniqueidprovider.h
 *
 *  Created on: Feb 16, 2016
 *      Author: janne
 */

#ifndef INC_UNIQUEIDPROVIDER_H_
#define INC_UNIQUEIDPROVIDER_H_

#include <mutex>

class UniqueIdProvider
{
public:
	static UniqueIdProvider* GetApi();
	uint32_t GetUniqueId();

protected:

private:
	UniqueIdProvider();
	static UniqueIdProvider* instance;
	static std::mutex instanceCreationMutex;
	std::mutex getIdMutex;
	uint32_t currentId;
};



#endif /* INC_UNIQUEIDPROVIDER_H_ */
