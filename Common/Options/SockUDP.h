#pragma once

#include "SockClient.h"

int sockUDPOpenSend(void);
int sockUDPOpenRecv(uint portno);
void sockUDPSendBlock(int sock, uchar ip[4], char *domain, uint portno, uchar *data, uint dataSize);
void sockUDPSend(int sock, uchar ip[4], char *domain, uint portno, autoBlock_t *block);
uint sockUDPRecvBlock(int sock, uint millis, uchar *buff, uint buffSize);
autoBlock_t *sockUDPRecv(int sock, uint millis);
void sockUDPClose(int sock);
