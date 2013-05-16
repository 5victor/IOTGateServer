/*
 * MT.cpp
 *
 *  Created on: Mar 27, 2013
 *      Author: victor
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <string.h>

#include "MT.h"
#include "debug.h"


MT::MT() {
	// TODO Auto-generated constructor stub
	mutexsend = new Mutex(Mutex::SHARED);
//	condsend = new Condition(SHARED);
	mutexcomp = new Mutex(Mutex::SHARED);
	condcomp = new Condition(Condition::SHARED);
	sreqsend = NULL;
}

MT::~MT() {
	// TODO Auto-generated destructor stub
}

int MT::start(ZNP *znp)
{
	int ret;

	this->znp = znp;

	ret = initUart();
	if (ret) {
		D("initUart return %d", ret);
		return ret;
	}

	this->run();
	return 0;
}

int MT::initUart(void)
{
	struct termios options;
	int ctrlbits;
	ufd = open("/dev/ttySAC3", O_RDWR | O_NOCTTY);
	if (ufd < 0)
		return ufd;

	ioctl(ufd, TIOCMGET, &ctrlbits);
	if (ctrlbits & TIOCM_CTS)
		D("%s:connect", __FUNCTION__);
	else
		D("%s:disconnect", __FUNCTION__);

	tcgetattr(ufd, &options);
	cfsetispeed(&options, B38400);
	cfsetospeed(&options, B38400);
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;
//	options.c_cflag |= CRTSCTS;
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG | IEXTEN); /*Control*/
	options.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	options.c_oflag &= ~OPOST;   /*Output*/

	tcsetattr(ufd, TCSANOW, &options);
	return 0;
}

uint8_t MT::calcFCS(uint8_t *pMsg,
		uint8_t len, uint8_t fcs)
{
//	uint8_t result = fcs;
	while(len--)
	{
		fcs ^= *pMsg++;
	}
	return fcs;
}

FRAME *MT::recvFrame()
{
#define BUF_SIZE 512
	int ret;
	int i;
	uint8_t ch;
	uint8_t buf[BUF_SIZE];
	int len;
	int num;
	int start;
	uint8_t fcs;
	FRAME *frame;

	D("%s", __FUNCTION__);

	do {
		ret = read(ufd, &ch, 1);
		if (ret == 1 && ch == 0xFE)
			break;
		else if (ret < 0)
			return NULL;
		D("%s:char != 0xFE", __FUNCTION__);
	} while(1);
	D("%s:recv 0xFE", __FUNCTION__);
	do {
		ret = read(ufd, &ch, 1);
		if (ret == 1) {
			len = ch;
			break;
		} else if (ret < 0)
			return NULL;
	} while (1);
	D("%s:len=%d", __FUNCTION__, len);
	num = len + 3; /* CMD0, CMD1, DATA, FCS */
	start = 0;
	do {
		ret = read(ufd, &buf[start], num);
		if (ret > 0) {
			start += ret;
			num -= ret;
			if (num)
				continue;
			else
				break;
		} else if (ret < 0)
			return NULL;
	} while(1);

	fcs = calcFCS((uint8_t *)&len, 1, 0); /* len */
	fcs = calcFCS(buf, len + 2, fcs); /* cmd0, cmd1, data */
	if (fcs != buf[len + 2]) {
		D("%s:FCS check fail, should %d, but %d", __FUNCTION__, fcs, buf[len + 2]);
		D("%s:bad packet len=%d, cmd0=0x%x, cmd1=0x%x", __FUNCTION__, len, buf[0], buf[1]);
		return NULL;
	}

	frame = new FRAME();
	frame->len = len;
	frame->cmd0 = buf[0];
	frame->cmd1 = buf[1];
	frame->data = new uint8_t[len];
	memcpy(frame->data, &buf[2], len);
	D("%s:Get one Frame len=%d,cmd0=0x%x,cmd1=0x%x", __FUNCTION__, frame->len, frame->cmd0, frame->cmd1);

	return frame;
}

bool MT::threadLoop()
{
	uint8_t buf[BUF_SIZE];
	bool receving;
	int ret;
	FRAME *frame;
	uint8_t fcs;
	int recevied;
	D("MT::threadLoop start");
	do {
			frame = recvFrame();
			if (frame != NULL)
				handleRecvFrame(frame);
	} while(1);

	return true;
}

void MT::handleRecvFrame(FRAME *frame)
{
	if (sreqsend) {
		if ((sreqsend->cmd0 & 0xF) == (frame->cmd0 & 0xF)) {
			if (sreqsend->cmd1 == frame->cmd1) {
				D("%s:SRSP cmd0=0x%x, cmd1=0x%x", __FUNCTION__, frame->cmd0, frame->cmd1);
				sreqresult = frame;
				condcomp->signal();
				return;
			}
		}
	}

	if ((frame->cmd0 & 0x40) == 0x40) {
		if (znp) {
			znp->handleAREQ(frame);
			return;
		} else {
			D("%s:areqhandle == NULL", __FUNCTION__);
		}
	}
	D("handle RecvFrame have unprocess frame");
	return;
}

FRAME *MT::sendSREQ(FRAME *send)
{
	FRAME *result;
	int ret;

	mutexsend->lock();

	ret = sendFrame(send);
	if (ret) {
		result = NULL;
		goto out;
	}
	sreqsend = send; //this var lock by mutexsend
	D("%s:send complete and have no error\n", __FUNCTION__);

	{
		Mutex::Autolock _l(*mutexcomp);
		condcomp->wait(*mutexcomp);
		D("%s: recv SRSP", __FUNCTION__);
	}
	//result = sreq.error ? NULL:sreq.result;
	result = sreqresult;
	sreqsend = NULL;
out:
	mutexsend->unlock();
//	delete sreq.frame->data;
//	delete sreq.frame;
//	sreq.senderror = false;
	return result;
}

int MT::sendFrame(FRAME *frame)
{
	int ret;
	uint8_t sof = 0xFE;
	uint8_t fcs;

	D("%s:enter", __FUNCTION__);

	ret = write(ufd, &sof, 1);
	if (ret != 1)
		goto error;
	ret = write(ufd, &frame->len, 1);
	if (ret != 1)
		goto error;
	ret = write(ufd, &frame->cmd0, 1);
	if (ret != 1)
		goto error;
	ret = write(ufd, &frame->cmd1, 1);
	if (ret != 1)
		goto error;

	if (frame->len) {
		ret = write(ufd, frame->data, frame->len);
		if (ret != frame->len)
			goto error;
	}

	fcs = calcFCS(&frame->len, 1, 0);
	fcs = calcFCS(&frame->cmd0, 1, fcs);
	fcs = calcFCS(&frame->cmd1, 1, fcs);
	if (frame->len)
		fcs = calcFCS(frame->data, frame->len, fcs);

	ret = write(ufd, &fcs, 1);
	if (ret < 0)
		goto error;

	return 0;

error:
	return ret;
}

int MT::sendAREQ(FRAME *send)
{
	int ret;

	MT::mutexsend->lock();

	ret = sendFrame(send);
    D("%s:complete", __FUNCTION__);
	mutexsend->unlock();
	return ret;
}
