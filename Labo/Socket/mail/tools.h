#pragma once

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\CryptoRand.h"

char *MakeMailMessageID(char *senderMailAddr);
char *GetMailHeader(autoList_t *mail, char *key);
char *RefMailHeader(autoList_t *mail, char *key);
autoList_t *GetMailBody(autoList_t *mail);

char *c_MakeMailMessageID(char *senderMailAddr);
char *c_RefMailHeader(autoList_t *mail, char *key);
