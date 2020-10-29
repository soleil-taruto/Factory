#pragma once

#include "CrossChannel.h"
#include "C:\Factory\Common\Options\SockServerTh.h"

extern uint PortNo;
extern char *FwdHost;
extern uint FwdPortNo;
extern uint ConnectMax;

extern void (*TunnelKeyEvent)(int key);
extern void (*TunnelPerformTh)(int sock, char *strip);

void TunnelMain(int (*userReadArgs)(void), void (*userPerform)(int sock, int fwdSock), char *title, char *(*cb_getTitleSuffix)(void));

// ---- Tools ----

int IsTight(void);

// ----
