#pragma once

#include "C:\Factory\Common\Options\SockStream.h"
#include "nSyncDefine.h"

extern char *NS_AppTitle;

void NS_DeletePath(char *path);
void NS_CreateParent(char *path);
void NS_SendFile(SockStream_t *ss, char *file);
void NS_RecvFile(SockStream_t *ss, char *file);
