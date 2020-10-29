#pragma once

#include "Socket.h"
#include "SockClient.h"

void sockMonitor(
	uint (*selectNicProc)(autoList_t *strNicIpList),
	void (*recvedDataProc)(uchar *recvedData, uint recvedSize),
	int (*idleProc)(void)
	);
