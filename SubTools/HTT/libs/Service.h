#pragma once

#include "C:\Factory\Common\all.h"

#define HTT_IP_FILE "IP.httdat"
#define HTT_RECV_FILE "Recv.httdat"
#define HTT_SEND_FILE "Send.httdat"
#define HTT_TIGHT_FILE "Tight.httdat"

extern char *HttIPFile;
extern char *HttRecvFile;
extern char *HttSendFile;
extern char *HttTightFile;
extern char *HttIP;

extern time_t BootTime;
extern time_t ConnectTime;
extern time_t LastRecvTime;
extern time_t LastSendTime;
extern time_t ConnectElapseTime;
extern time_t LastRecvElapseTime;
extern time_t LastSendElapseTime;

void ServiceDisconnect(void);

void ServiceMain(void);   // 要実装, 送受信メイン
void ServicePeriod(void); // 要実装, 定期的な空実行
