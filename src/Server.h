/*
 * Server.h
 *
 *  Created on: Mar 29, 2013
 *      Author: victor
 */

#ifndef SERVER_H_
#define SERVER_H_

#include <utils/Thread.h>
using namespace android;

#include <vector>
using namespace std;
#include "Node.h"
#include "ZNP.h"
#include "SocketServer.h"

class ZNP;

class SocketServer;

class Server : public Thread {
public:
	Server();
	virtual ~Server();
	int initServer();
	void startServer();
	ZNP *getZNP();
	int startNetwork(int panid, unsigned int chanlist = 0x00008000);
	void setSocketServer(SocketServer *ss);
	int getNodeNum();
	Node *getNode(int i);
	Node *getNode(uint16_t nwkaddr);

	int sendClusterData(struct cluster_data *cd, SocketSession *session);
	void recvClusterData(struct cluster_data *cd);

//commit command, run in threadLoop()
public:
	void foundNode(uint16_t nwkaddr);

private:
	int addNode(uint16_t nwkaddr);
	int getNodeEndpoints(uint16_t nwkaddr, vector<Endpoint *> &endpoints);
	Endpoint *getEndpoint(uint16_t nwkaddr, int index);

private:
	int getNodelist(vector<uint16_t> list);

private:
	vector<Node *> nodes;
	ZNP *znp;
	SocketServer *socketserver;
	struct cluster_session *sendcs; //bad imp, use list replace

private:
	enum Command {FOUND_NODE,};
	Mutex *mutexcmd;
	Condition *condcmd;

	Mutex *cmdlock;

	enum Command cmd;
	struct {
		union {
			unsigned int udata;
			int sdata;
			unsigned char *buf;
		};
		int len;
	}cmddata;

	bool threadLoop();
	void commitCommand(enum Command cmd);
	void setSignData(int data);
	void setUnsignData(unsigned int data);
	void setBufData(unsigned char *buf, int len);
};

#endif /* SERVER_H_ */
