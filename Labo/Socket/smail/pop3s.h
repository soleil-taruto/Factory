#pragma once

#include "C:\Factory\Common\all.h"
#include "Common.h"

autoList_t *GetMailList(char *popServer, uint portno, char *user, char *pass);
autoBlock_t *RecvMail(char *popServer, uint portno, char *user, char *pass, uint mailno, uint mailSizeMax);
void DeleteMail(char *popServer, uint portno, char *user, char *pass, uint mailno);
