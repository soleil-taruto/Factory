#pragma once

#include "C:\Factory\Common\Options\SockStream.h"

typedef struct httpChunkedRecver_st // static member
{
	SockStream_t *Stream;
	uint RemSize;
}
httpChunkedRecver_t;

httpChunkedRecver_t *httpCreateChunkedRecver(SockStream_t *ss);
void httpReleaseChunkedRecver(httpChunkedRecver_t *i);
autoBlock_t *httpRecvChunked(httpChunkedRecver_t *i);
