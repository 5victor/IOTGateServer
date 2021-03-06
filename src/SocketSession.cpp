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
	return readData((uint8_t *)h, sizeof(struct hdr));
}

int SocketSession::readData(void *buf, int num)
{
	int ret;
	int readed;
	uint8_t *p = (uint8_t *)buf;;
	readed = 0;
	while(1) {
		ret = read(afd, &p[readed], num - readed);
		if (!ret)
			return -1;

		readed += ret;
		if (readed == num)
			break;
	}

	return readed;
}

int SocketSession::writeData(void *buf, int num)
{
	int ret;
	int written;
	uint8_t *p = (uint8_t *)buf;;
	written = 0;
	while(1) {
		ret = write(afd, &p[written], num - written);
		if (!ret)
			return -1;

		written += ret;
		if (written == num)
			break;
	}

	return written;
}

int SocketSession::writeHead(struct hdr *h)
{
	return writeData(h, sizeof(struct hdr));
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
		if (ret < 0) {
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
		case QUERY_NODE_ENDPOINTS:
			handleQueryEndpoints(hdr);
			break;
        case SEND_CLUSTER_DATA:
            handleSendClusterData(hdr);
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

	ret = writeHead(&hdr);
	if (ret < 0)
		LOG("%s:write Head error", __FUNCTION__);
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
		Node *Node = server->getNode(i);
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

void SocketSession::handleQueryEndpoints(struct hdr hdr)
{
	uint16_t nwkaddr;
	int ret;
	int i;

	ret = readData((uint8_t *)&nwkaddr, sizeof(uint16_t));

	if (ret != hdr.data_len) {
		LOG("%s:packet error data_len=%d", __FUNCTION__, hdr.data_len);
		hdr.data_len = 0;
		writeHead(&hdr);
		return ;
	}

	Node *node = server->getNode(nwkaddr);
	vector<Endpoint *>endpoints = node->getEndpoints();

	hdr.data_len = endpoints.size() * sizeof(struct endpoint);
	ret = writeHead(&hdr);
	if (ret < 0)
		LOG("%s:writeHead error", __FUNCTION__);

	for (i = 0; i < endpoints.size(); i++) {
		Endpoint *Ep = endpoints.at(i);
		struct endpoint ep;
		ep.index = Ep->getIndex();
		ep.nwkaddr = Ep->getNWKAddr();
		ep.profileid = Ep->getProfileID();
		ep.deviceid = Ep->getDeviceID();
		ep.inclusternum = Ep->getInClusters(ep.inclusterlist);
		ep.outclusternum = Ep->getOutClusters(ep.outclusterlist);
		ret = writeData(&ep, sizeof(struct endpoint));
		if (ret < 0) {
			LOG("%s:write endpoint %d fail", __FUNCTION__, i);
		}
	}
}

void SocketSession::handleSendClusterData(struct hdr hdr)
{
    struct cluster_hdr chdr;
    struct cluster_data cd;
    int ret;
    void *data;

    ret = readData(&chdr, sizeof(struct cluster_hdr));
    if (ret < 0) {
        D("%s:read cluster_hdr fail", __FUNCTION__);
        return ;
    }

    cd.nwkaddr = chdr.nwkaddr;
    cd.cluster = chdr.cluster;
    cd.srcep = chdr.srcep;
    cd.dstep = chdr.dstep;
    cd.len = chdr.data_len;

    if (chdr.data_len > 0) {
        data = malloc(chdr.data_len);
        if (data == NULL)
            return;
        ret = readData(data, cd.len);
        if (ret < 0) {
            D("%s:read data fail", __FUNCTION__);
            free(data);
        }
    } else {
        cd.data = NULL;
    }
    ret = server->sendClusterData(&cd, this);
    if (ret) {
        D("%s:sendClusterData fail", __FUNCTION__);
    }
}

void SocketSession::recvClusterData(struct cluster_data *cd)
{
    struct hdr hdr;
    struct cluster_hdr chdr;
    int ret;
    D("SocketSession::%s:nwkaddr=0x%x,cluster=%d,dstep=%d,srcep=%d,len=%d", __FUNCTION__, cd->nwkaddr, cd->cluster, cd->dstep, cd->srcep, cd->len);
    hdr.token = token;
    hdr.cmd = SEND_CLUSTER_DATA;
    hdr.data_len = 0;

    writeHead(&hdr);

    chdr.nwkaddr = cd->nwkaddr;
    chdr.cluster = cd->cluster;
    chdr.dstep = cd->dstep;
    chdr.srcep = cd->srcep;
    chdr.data_len = cd->len;

    writeData(&chdr, sizeof(struct cluster_hdr));
    D("%s:write cluster_hdr ok", __FUNCTION__);

    if (chdr.data_len > 0) {
        writeData(cd->data, cd->len);
    }
    D("%s:handle ok", __FUNCTION__);
}
