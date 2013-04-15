/*
 * SocketSession.h
 *
 *  Created on: Apr 8, 2013
 *      Author: victor
 */

#ifndef SOCKETSESSION_H_
#define SOCKETSESSION_H_

#include <openssl/ssl.h>
#include <utils/Thread.h>
using namespace android;

#include "SocketServer.h"

enum {
	//QUERY_NODE_NUM = 0x5,
	GET_TOKEN = 0x5, //client to server
	QUERY_NODES, //c2s
	QUERY_NODE_ENDPOINTS, //c2s
	SEND_CLUSTER_DATA, //s2c c2s
	ADD_NODES, //s2c
};

struct node {
	unsigned short nwkaddr;
	int type;
	int epnum;
	unsigned char ieeeaddr[8];
}__attribute__ ((packed));

struct endpoint {
	uint8_t index;
	uint16_t nwkaddr;
	uint16_t profileid;
	uint16_t deviceid;
	uint8_t inclusternum;
	uint16_t inclusterlist[MAX_CLUSTER];
	uint8_t outclusternum;
	uint16_t outclusterlist[MAX_CLUSTER];
}__attribute__ ((packed));

struct cluster_hdr {
	uint16_t cluster;
	int		data_len;
}__attribute__ ((packed));

//#define SOF		'V'
//#define EOF		'W'

struct hdr {
	uint8_t token;
	uint8_t cmd;
//	uint8_t id;
	int	data_len;
}__attribute__ ((packed));

class SocketServer;

class SocketSession : public Thread {
public:
	SocketSession();
	virtual ~SocketSession();
	void init(SSL *ssl, SocketServer *server);
	void start();

private:
	SSL *ssl;
	SocketServer *socketserver;
	uint8_t token;

private:
	bool threadLoop();

private:
	void handleGetToken(struct hdr hdr);
};

#endif /* SOCKETSESSION_H_ */
