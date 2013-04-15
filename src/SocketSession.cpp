/*
 * SocketSession.cpp
 *
 *  Created on: Apr 8, 2013
 *      Author: victor
 */

#include "SocketSession.h"
#include <time.h>
#include <stdlib.h>

SocketSession::SocketSession() {
	// TODO Auto-generated constructor stub

}

SocketSession::~SocketSession() {
	// TODO Auto-generated destructor stub
	D("%s:called", __PRETTY_FUNCTION__);
	SSL_free(ssl);
//	socketserver->freeSocketSession(this); //在delete时才会调用析构函数吧
}

void SocketSession::init(SSL *ssl, SocketServer *server)
{
	this->ssl = ssl;
	this->socketserver = server;
}

void SocketSession::start()
{
	this->run();
}

bool SocketSession::threadLoop()
{
	LOG("enter %s", __PRETTY_FUNCTION__);
	struct hdr hdr;
	int ret;
	//uint8_t sof;

	do {
		/*
		ret = SSL_read(ssl, &sof, 1);
		if (ret != 1 || sof != SOF)
			continue;
			*/

		ret = SSL_read(ssl, &hdr, sizeof(hdr));
		D("%s:recv cmd=%d,len=%d", __FUNCTION__, hdr.cmd, hdr.data_len);
		switch (hdr.cmd) {
		case GET_TOKEN:
			handleGetToken(hdr);
			break;
		default:
			LOG("%s:unhandel packet cmd=%d len=%d", __PRETTY_FUNCTION__, hdr.cmd, hdr.data_len);
		};

	}while (1);

	socketserver->freeSocketSession(this);
	return true;
}

void SocketSession::handleGetToken(struct hdr hdr)
{
	int ret;

	srandom(time(NULL));
	token = random();

	hdr.token = token;
	hdr.cmd = GET_TOKEN;
	hdr.data_len = 0;

	ret = SSL_write(ssl, &hdr, sizeof(struct hdr));
	if (ret != sizeof(struct hdr)) {
		LOG("%s:SSL_write fail %d", __FUNCTION__, ret);
	}
}
