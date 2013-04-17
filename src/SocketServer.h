/*
 * SocketServer.h
 *
 *  Created on: Apr 7, 2013
 *      Author: victor
 */

#ifndef SOCKETSERVER_H_
#define SOCKETSERVER_H_

#include <utils/Thread.h>
using namespace android;

#include <openssl/ssl.h>
#include <vector>
#include "Server.h"
#include "SocketSession.h"

class Server;

class SocketSession;

class SocketServer : public Thread {
friend class SocketSession;
public:
	SocketServer();
	virtual ~SocketServer();
	int init(Server *server);
	void start();
	int getState();


public:
	void setPort(int port);

private:
	int sfd;
	//int afd;
	int port;
	Server *server;
enum state {
	UNINIT = 0,
	INITED,
	CONNECTED,
};
	int state;

private:
	vector<SocketSession *> sessions;
	int newSocketSession(int fd);
	void freeSocketSession(SocketSession *session);

private:
	int initSocket();

public:
	bool threadLoop();
};

#endif /* SOCKETSERVER_H_ */
