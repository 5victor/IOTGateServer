/*
 * SocketSession.cpp
 *
 *  Created on: Apr 8, 2013
 *      Author: victor
 */

#include <time.h>
#include <stdlib.h>
#include "SocketSession.h"

SocketSession::SocketSession() {
	// TODO Auto-generated constructor stub

}

SocketSession::~SocketSession() {
	// TODO Auto-generated destructor stub
	D("%s:called", __PRETTY_FUNCTION__);
//	socketserver->freeSocketSession(this); //在delete时才会调用析构函数吧
}

void SocketSession::init(int fd, SocketServer *server)
{
	afd = fd;
	this->socketserver = server;
	this->server = socketserver->server;
}

void SocketSession::start()
{
	this->run();
}

int SocketSession::readHead(struct hdr *h)
{
	int ret;
	int readed;
	uint8_t *p = (uint8_t *)h;
	readed = 0;
	while(1) {
		ret = read(afd, &p[readed], sizeof(struct hdr) - readed);
		if (!ret)
			return -1;

		readed += ret;
		if (readed == sizeof(struct hdr))
			break;
	}

	return 0;
}

void SocketSession::flushData(struct hdr *h)
{
	int i;
	int ret;

	for (i = 0; i < h->data_len; i++) {
		int buf;
		ret = read(afd, &buf, 1);
		if (ret <= 0)
			return;
	}
}

bool SocketSession::threadLoop()
{
	LOG("enter %s", __PRETTY_FUNCTION__);
	struct hdr hdr;
	int ret;
	//uint8_t sof;

	do {

		ret = readHead(&hdr);
		D("%s:socket read return %d", __PRETTY_FUNCTION__, ret);
		D("%s:recv token=%d,cmd=%d,len=%d", __FUNCTION__, hdr.token, hdr.cmd, hdr.data_len);
		if (ret) {
			break;
		}
		switch (hdr.cmd) {
		case GET_TOKEN:
			handleGetToken(hdr);
			break;
		case QUERY_NODE_NUM:
			handleQueryNodeNum(hdr);
			break;
		case QUERY_NODES:
			handleQueryNodes(hdr);
			break;
		default:
			LOG("%s:unhandel packet cmd=%d len=%d", __PRETTY_FUNCTION__, hdr.cmd, hdr.data_len);
		};

	}while (1);

	//socketserver->freeSocketSession(this);
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

	ret = write(afd, &hdr, sizeof(struct hdr));
	if (ret != sizeof(struct hdr)) {
		LOG("%s:write fail %d", __FUNCTION__, ret);
	}
}

void SocketSession::handleQueryNodeNum(struct hdr h)
{
	uint32_t nodeNum;
	int ret;

	struct hdr hdr = h;
	hdr.data_len = 4;

	ret = write(afd, &hdr, sizeof(struct hdr));
	if (ret != sizeof(struct hdr)) {
		LOG("%s:write hdr fail %d", __FUNCTION__, ret);
	}

	nodeNum = server->getNodeNum();
	D("%s:data_len %d, nodeNum %d", __FUNCTION__, hdr.data_len, nodeNum);
	ret = write(afd, &nodeNum, sizeof(uint32_t));

	if (ret != sizeof(uint32_t))
		LOG("%s:write hdr fail %d", __FUNCTION__, ret);
}

void SocketSession::handleQueryNodes(struct hdr hdr)
{
	int ret;
	struct node node;
	int nodeNum = server->getNodeNum();
	hdr.data_len = nodeNum * sizeof(struct node);

	ret = write(afd, &hdr, sizeof(struct hdr));
	if (ret != sizeof(struct hdr)) {
		LOG("%s:write hdr fail %d", __FUNCTION__, ret);
	}

	for (int i = 0; i < nodeNum; i++) {
		struct Node *Node = server->getNode(i);
		node.nwkaddr = Node->getNWKAddr();
		node.type = Node->getType();
		node.epnum = Node->getEndpoints().size();
		memcpy(node.ieeeaddr, Node->getIEEEAddr(), 8);
		ret = write(afd, &node, sizeof(struct node));
		if (ret != sizeof(struct node)) {
			LOG("%s:write node %d fail", __FUNCTION__, i);
		}
	}
}
