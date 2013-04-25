/*
 * MT.h
 *
 *  Created on: Mar 27, 2013
 *      Author: victor
 */

#ifndef MT_H_
#define MT_H_

#include <utils/Thread.h>
#include <utils/Mutex.h>
using namespace android;

#include "common.h"
#include "ZNP.h"

/*
struct SRSP {
	uint8_t len;
	uint8_t cmd0;
	uint8_t cmd1;
	union {
		uint8_t status;
		uint8_t *data;
	};
};
*/

class ZNP;

class MT : public Thread {
public:
	MT();
	int start(ZNP *znp);
	virtual ~MT();
	FRAME *sendSREQ(FRAME *send);
	int sendAREQ(FRAME *send);

private:
	ZNP *znp;

private:
	int ufd;
	Mutex *mutexsend;
	//static Condition *condsend;
	Mutex *mutexcomp;
	Condition *condcomp;

	FRAME *sreqresult;
	FRAME *sreqsend;

	bool threadLoop();

private:
	int initUart(void);
	int initSignal(void);
	uint8_t calcFCS(uint8_t *pMsg,
			uint8_t len, uint8_t fcs);
	void handleRecvFrame(FRAME *frame);
	FRAME *recvFrame();
	int sendFrame(FRAME *frame);
};

#endif /* MT_H_ */
