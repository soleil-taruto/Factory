#pragma once

#include "C:\Factory\Common\all.h"
#include "Common.h"

extern int SendMailLastErrorFlag;

void SendMail(char *smtpServer, uint portno, char *user, char *pass, char *fromMailAddress, char *toMailAddress, autoBlock_t *mail);
