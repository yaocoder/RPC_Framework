/*
 * defines.h
 *
 *  Created on: 2012-9-12
 *      Author: yaowei
 */

#ifndef DEFINES_H_
#define DEFINES_H_

#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <list>
#include <event.h>

#define DATA_BUFFER_SIZE 10240
#define CRLF "\r\n"

typedef struct
{
	char buf[DATA_BUFFER_SIZE];
	unsigned int len;
	evutil_socket_t  sfd;
	int  flag;
	struct event_base 	*base;
}LOCAL_REV_DATA;

typedef struct
{
	char recv_buf[100];
	char name_buf[64];
	unsigned int len;
	int  sfd;
}MAIL_OPT_DATA;

struct ClientInfo
{
	int sfd;
	struct sockaddr_in clientAddr;
};

enum
{
	STATS_PROCESS,
	LOGIC_PROCESS,
};


#endif /* DEFINES_H_ */
