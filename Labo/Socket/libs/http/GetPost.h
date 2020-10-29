#pragma once

#include "C:\Factory\Common\Options\SockStream.h"
#include "Chunked.h"
#include "ExtToContentType.h"

extern autoList_t *httpRecvedHeader;

extern int httpRecvRequestFirstWaitDisable;
extern uint httpRecvRequestFirstWaitMillis;
extern char *httpRecvRequestHostValue;

void httpRecvRequestHeader(SockStream_t *i, char **pHeader, int *pChunked, uint *pContentSize);
void httpRecvRequest(SockStream_t *i, char **pHeader, uchar **pContent, uint *pContentSize);
void httpRecvRequestMax(SockStream_t *i, char **pHeader, uchar **pContent, uint *pContentSize, uint contentSizeMax);
void httpSendResponseHeader(SockStream_t *i, uint64 cSize, char *contentType);
void httpSendResponse(SockStream_t *i, autoBlock_t *content, char *contentType);
void httpSendResponseText(SockStream_t *i, char *text, char *contentType);

typedef struct httpDecode_st
{
	char *Url;
	char *Query;
	char *Path;

	// ˆÈ‰º URL-decoded
	char *DecPath;
	autoList_t *DirList;
	autoList_t *Keys;
	autoList_t *Values;
}
httpDecode_t;

void httpUrlDecoder(char *line);
char *httpUrlEncoder(char *line);
void httpDecodeUrl(char *url, httpDecode_t *out);
void httpDecodeQuery(char *query, httpDecode_t *out);
void httpDecode(char *header, uchar *content, httpDecode_t *out);
void httpDecodeFree(httpDecode_t *out);

void httpDecodeHeader(char *header, httpDecode_t *out);
void httpDecodeHeaderFree(httpDecode_t *out);
void httpDecodeQuery(char *query, httpDecode_t *out);
void httpDecode(char *header, uchar *content, httpDecode_t *out);
void httpDecodeFree(httpDecode_t *out);

#define HTTPSENDRESPONSEFILETYPE_FILEIO_MUTEXNAME "cerulean.charlotte Factory httpSendResponseFileType file-io mutex object"

void httpSendResponseFileType(SockStream_t *i, char *file, char *contentType);
void httpSendResponseFile(SockStream_t *i, char *file);
