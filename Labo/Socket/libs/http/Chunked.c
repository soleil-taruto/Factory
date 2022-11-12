#include "Chunked.h"

#define RECVSIZE_ONCE_MAX (1024 * 1024 * 16)

httpChunkedRecver_t *httpCreateChunkedRecver(SockStream_t *ss)
{
	httpChunkedRecver_t *i = (httpChunkedRecver_t *)memAlloc(sizeof(httpChunkedRecver_t));

	i->Stream = ss;
	i->RemSize = 0;

	return i;
}
void httpReleaseChunkedRecver(httpChunkedRecver_t *i)
{
	memFree(i);
}
static void RecvChunkedTrailer(httpChunkedRecver_t *i)
{
	for (; ; )
	{
		char *line = SockRecvLine(i->Stream, 1000);

		if (!*line)
		{
			memFree(line);
			break;
		}
		memFree(line);
	}
}
autoBlock_t *httpRecvChunked(httpChunkedRecver_t *i) // ret == NULL: �I��
{
	uint rSize;
	void *rBlock;

	errorCase(!i);
	errorCase(!i->Stream); // ? ���ɏI��

	if (!i->RemSize)
	{
		char *line = SockRecvLine(i->Stream, 1000);
		// �ő�W���������ǁA���s�Ƃ����邵�M���M���ɂ���K�v���Ȃ����ȂƁB<- chunk-extension �����邶��Ȃ��́B30 -> 1000

		strchrEnd(line, ';')[0] = '\0'; // chunk-extension �̔r��

		i->RemSize = toValueDigits(line, hexadecimal);
		memFree(line);

		if (!i->RemSize)
		{
			RecvChunkedTrailer(i);

			i->Stream = NULL;
			return NULL;
		}
	}
	rSize = m_min(RECVSIZE_ONCE_MAX, i->RemSize);
	rBlock = memAlloc(rSize);

	SockRecvBlock(i->Stream, rBlock, rSize);
	i->RemSize -= rSize;

	if (!i->RemSize)
	{
		// CR-LF
		SockRecvChar(i->Stream);
		SockRecvChar(i->Stream);
	}
	return bindBlock(rBlock, rSize);
}
