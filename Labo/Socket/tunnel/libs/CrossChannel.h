#pragma once

#include "C:\Factory\Common\Options\SockClient.h" // •K—v‚È‚Ì‚Í Socket.h
#include "CCCCommon.h"

extern int ProcDeadFlag;
extern int ChannelDeadFlag;

void CrossChannel(
	int sockA,
	int sockB,
	void (*aToBFltr)(autoBlock_t *, uint),
	uint aToBUserData,
	void (*bToAFltr)(autoBlock_t *, uint),
	uint bToAUserData
	);
