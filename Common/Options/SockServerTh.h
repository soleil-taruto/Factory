#pragma once

#include "Socket.h"
#include "SockConnectMon.h"
#include "Thread.h"
#include "ConsoleColor.h"

extern uint sockConnectMax;
extern uint sockConnectedCount;
extern double sockConnectedRate;

void sockServerTh(void (*funcTransmit)(int, char *), uint portno, uint connectmax, int (*funcIdle)(void));
