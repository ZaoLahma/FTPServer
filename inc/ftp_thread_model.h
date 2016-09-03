/*
 * ftp_thread_model.h
 *
 *  Created on: Sep 3, 2016
 *      Author: janne
 */

#ifndef INC_FTP_THREAD_MODEL_H_
#define INC_FTP_THREAD_MODEL_H_

/*
 * This file defines the FTP thread model.
 */

static const uint32_t SOCKET_LISTENER_THREAD_GROUP_ID = 3;
static const uint32_t SOCKET_LISTENER_THREAD_GROUP_MAX_NO_OF_THREADS = 1;

static const uint32_t DATA_CHANNEL_THREAD_GROUP_ID = 4;
static const uint32_t DATA_CHANNEL_THREAD_GROUP_MAX_NO_OF_THREADS = 0; //0 == unlimited

static const uint32_t ADMIN_THREAD_GROUP_ID = 5;
static const uint32_t ADMIN_THREAD_GROUP_MAX_NO_OF_THREADS = 1;
#endif /* INC_FTP_THREAD_MODEL_H_ */
