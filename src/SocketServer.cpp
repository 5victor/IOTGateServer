/*
 * SocketServer.cpp
 *
 *  Created on: Apr 7, 2013
 *      Author: victor
 */

#include "SocketServer.h"

#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

SocketServer::SocketServer() {
	// TODO Auto-generated constructor stub
	port = 1013;
	state = UNINIT;
}

SocketServer::~SocketServer() {
	// TODO Auto-generated destructor stub
}

int SocketServer::init(Server *server)
{
	int ret;
	this->server = server;
	ret = initSocket();
	if (ret)
		return ret;

	return ret;
}

void SocketServer::start()
{
	this->run();
}

int SocketServer::initSocket()
{
	struct sockaddr_in serv_addr;
	int ret;

	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd < 0) {
		D("%s:create socket fail %d",__FUNCTION__ , sfd);
		return sfd;
	}

	memset(&serv_addr, 0x0, sizeof(struct sockaddr_in));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);
    ret = bind(sfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in));
    if (ret) {
    	D("%s:bind fail %d", __FUNCTION__, ret);
    	return ret;
    }

    state = INITED;

	return 0;
}

int SocketServer::newSocketSession(int fd)
{
	SocketSession *ss = new SocketSession();

	if (ss == NULL)
		return -1;
	ss->init(fd, this);
	sessions.push_back(ss);
	LOG("start a new SocketSession");
	ss->start();

	return 0;
}

void SocketServer::freeSocketSession(SocketSession *session)
{
	vector<SocketSession *>::iterator it;
	for (it = sessions.begin() ; it != sessions.end(); ++it) {
		if ((*it) == session) {
			sessions.erase(it);
		}
	}
	//delete session; SocketSession inherit Thread, Thread auto delete
}

bool SocketServer::threadLoop()
{
	struct sockaddr_in addr;
	socklen_t addr_len;
	int ret;

	LOG("enter %s", __PRETTY_FUNCTION__);
	do {
		ret = listen(sfd, 5);
		int afd = accept(sfd, (struct sockaddr *)&addr, &addr_len);
		if (afd < 0) {
			LOG("%s:accept fail %d", __PRETTY_FUNCTION__, afd);
			continue;
		}
//		fcntl(afd,F_SETFL,fcntl(afd,F_GETFL,0) | O_NONBLOCK);
		D("%s:connect from %s", __PRETTY_FUNCTION__, inet_ntoa(addr.sin_addr));

		newSocketSession(afd);
	} while (1);


	return true;
}

