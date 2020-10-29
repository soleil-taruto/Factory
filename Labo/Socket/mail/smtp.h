#pragma once

#include "C:\Factory\Common\Options\SockClient.h"
#include "C:\Factory\Common\Options\SockStream.h"
#include "mutex.h"

int sendMail(char *smtpServer, uint portno, char *sendFrom, char *sendTo, autoList_t *mail);
int sendMailEx(char *smtpServer, uint portno, char *sendFrom, char *sendTo, autoList_t *mail, uint retrycnt, uint retryWaitMillis);
int sendMailEx2(char *smtpServer, uint portno, char *sendFrom, char *sendTo, autoList_t *mail);
