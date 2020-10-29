#pragma once

#include "GetPost.h"

typedef struct httpPart_st
{
	char *Name;      // line2JLine(, 1, 0, 0, 1) する。{ 未定義 or 空文字列 } -> 適当な文字列
	char *LocalFile; // line2JLine(, 1, 0, 0, 1) する。{ 未定義 or 空文字列 } -> 適当な文字列
	char *BodyFile;  // makeTempPath(), 空のcontentなら、0バイトのファイルになる。
}
httpPart_t;

extern uint httpMultiPartContentLenMax;
extern int httpM4UServerMode;
void httpRecvRequestFile(SockStream_t *i, char **pHeader, char *contentFile);

autoList_t *httpDivideContent(char *contentFile);
void httpReleaseParts(autoList_t *parts);
httpPart_t *httpGetPart(autoList_t *parts, char *name);

autoList_t *httpRecvRequestMultiPart(SockStream_t *i, char **pHeader);

char *httpGetPartLine(autoList_t *parts, char *name);
char *c_httpGetPartLine(autoList_t *parts, char *name);
