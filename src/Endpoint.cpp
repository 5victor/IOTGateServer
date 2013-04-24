/*
 * Endpoint.cpp
 *
 *  Created on: Mar 29, 2013
 *      Author: victor
 */

#include "Endpoint.h"
#include <string.h>

Endpoint::Endpoint() {
	// TODO Auto-generated constructor stub

}

Endpoint::~Endpoint() {
	// TODO Auto-generated destructor stub
}


void Endpoint::setIndex(uint8_t index)
{
	D("%s:index=0x%02x", __FUNCTION__, index);
	this->index = index;
}

uint8_t Endpoint::getIndex()
{
	D("%s:index=0x%02x", __FUNCTION__, index);
	return index;
}

void Endpoint::setNWKAddr(uint16_t nwkaddr)
{
	D("%s:nwkaddr=0x%04x", __FUNCTION__, nwkaddr);
	this->nwkaddr = nwkaddr;
}

uint16_t Endpoint::getNWKAddr()
{
	D("%s:nwkaddr=0x%04x", __FUNCTION__, nwkaddr);
	return nwkaddr;
}

void Endpoint::setProfileID(uint16_t profileid)
{
	D("%s:pid=0x%04x", __FUNCTION__, profileid);
	this->profileid = profileid;
}

uint16_t Endpoint::getProfileID()
{
	D("%s:pid=0x%04x", __FUNCTION__, profileid);
	return profileid;
}

void Endpoint::setDeviceID(uint16_t deviceid)
{
	D("%s:deviceid=0x%04x", __FUNCTION__, deviceid);
	this->deviceid = deviceid;
}

uint16_t Endpoint::getDeviceID()
{
	return deviceid;
}

void Endpoint::setInClusters(int num, uint16_t list[])
{
	D("%s:num=0x%04x", __FUNCTION__, num);
	this->inclusternum = num;
	memcpy(this->inclusterlist, list, sizeof(uint16_t) * num);
}

int Endpoint::getInClusters(uint16_t list[])
{
	memcpy(list, this->inclusterlist, sizeof(uint16_t) * inclusternum);
	return inclusternum;
}

void Endpoint::setOutClusters(int num, uint16_t list[])
{
	D("%s:num=0x%04x", __FUNCTION__, num);
	this->outclusternum = num;
	memcpy(this->outclusterlist, outclusterlist, sizeof(uint16_t) * num);
}

int Endpoint::getOutClusters(uint16_t list[])
{
	memcpy(list, this->outclusterlist, sizeof(uint16_t) * outclusternum);
	return outclusternum;
}
