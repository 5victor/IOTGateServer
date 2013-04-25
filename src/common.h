/*
 * common.h
 *
 *  Created on: Mar 27, 2013
 *      Author: victor
 */

#ifndef COMMON_H_
#define COMMON_H_

struct FRAME {
	unsigned char len; /* length of data */
	unsigned char cmd0;
	unsigned char cmd1;
	unsigned char *data;
};

void inline freeFrame(FRAME *frame)
{
	delete frame->data;
	delete frame;
}

struct cluster_data {
	uint16_t nwkaddr;
	uint8_t transid;
	int8_t dstep;
	int8_t srcep;
	uint16_t cluster;
	uint8_t len;
	uint8_t *data;
};

void inline freeClusterData(struct cluster_data *cd)
{
	if (cd->data)
		delete cd->data;
	delete cd;
}

struct cluster_session {
	struct cluster_data *data;
	SocketSession *session;
};

/////////////////////////////////////////////////////////////

#define LOG_TAG "IOTGateServer"

#define LOG_NDEBUG 0

#define LOG_NDEBUG_FUNCTION

#ifndef LOG_NDEBUG_FUNCTION
#define D(...) ((void)0)
#else
#define D(...) (ALOGV(__VA_ARGS__))
#endif

#define LOG(...) (ALOGV(__VA_ARGS__))

#include <cutils/log.h>

/////////////////////////////////////////////////////////////

#define MAX_CLUSTER 32

#endif /* COMMON_H_ */
