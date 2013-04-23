/*
 * SocketSession.h
 *
 *  Created on: Apr 8, 2013
 *      Author: victor
 */

#ifndef SOCKETSESSION_H_
#define SOCKETSESSION_H_

#include <utils/Thread.h>
using namespace android;

#include "SocketServer.h"
#include "Server.h"
#include "protocol.h"

class SocketServer;
class Server;

class SocketSession : public Thread {
public:
	SocketSession();
	virtual ~SocketSession();
	void init(int fd, SocketServer *server);
	void start();

private:
	int afd;
	SocketServer *socketserver;
	Server *server;
	uint8_t token;

private:
	bool threadLoop();
	int readHead(struct hdr *h);
	void flushData(struct hdr *h);
	int readData(void *buf, int num);
	int writeData(void *buf, int num);
	int writeHead(struct hdr *h);

private:
	void handleGetToken(struct hdr hdr);
	void handleQueryNodeNum(struct hdr hdr);
	void handleQueryNodes(struct hdr hdr);
	void handleQueryEndpoints(struct hdr hdr);
};

#endif /* SOCKETSESSION_H_ */
