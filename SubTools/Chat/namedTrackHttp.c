/*
	namedTrackHttp.exe 受信ポート 転送先ホスト 転送先ポート [/S]

		/S ... 停止
*/

#include "C:\Factory\Labo\Socket\tunnel\libs\Tunnel.h"
#include "C:\Factory\Common\Options\URL.h"

#define HEADER_FIRST_LINE_LENMAX 18000

typedef struct ChInfo_st
{
	int HeaderSend;
	autoBlock_t *RecvBuff;
}
ChInfo_t;

static char *ChangeHeader(char *header) // ret: ? NULL == 失敗
{
	autoList_t *tokens = tokenize(header, ' ');
	char *method;
	char *url;
	char *httpVer;
	char *nameBgn;
	char *nameEnd;
	char *name;
	char *url302Pfx;
	char *ret = NULL;

	method  = refLine(tokens, 0);
	url     = refLine(tokens, 1);
	httpVer = refLine(tokens, 2);

	url = urlDecoder(url);

	if (!*url)
		goto endfunc;

	if (startsWith(url, "http://"))
	{
		nameBgn = strchr(url + 7, '/');

		if (!nameBgn)
			goto endfunc;

		nameBgn++;
	}
	else
	{
		nameBgn = url + 1;
	}
	nameEnd = strchr(nameBgn, '/');

	if (!nameEnd)
		goto endfunc;

	*nameEnd = '\0';
	name = strx(nameBgn);
	toUpperLine(name); // HACK
	copyLine(nameBgn, nameEnd + 1);

	url = urlEncoder_x(url);
	url302Pfx = urlEncoder(name);

//	ret = xcout("%s\r\n%s %s %s", name, method, url, httpVer);
	ret = xcout("%s\r\n%s %s %s URL302PFX=/%s", name, method, url, httpVer, url302Pfx);

	memFree(name);
	memFree(url302Pfx);
endfunc:
	releaseDim(tokens, 1);
	memFree(url);
	return ret;
}
static uint GetIndexOf(autoBlock_t *buff, int chr)
{
	uint index;

	for (index = 0; index < getSize(buff); index++)
		if (getByte(buff, index) == chr)
			break;

	return index;
}
static void HttpNamedChannelFltr(autoBlock_t *buff, uint prm)
{
	ChInfo_t *i = (ChInfo_t *)prm;
	char *header;
	char *p;

	if (i->HeaderSend)
		return;

	ab_addBytes(i->RecvBuff, buff);
	setSize(buff, 0);
	header = ab_toLine(i->RecvBuff);
	p = strchr(header, '\r');

	if (!p)
	{
		if (HEADER_FIRST_LINE_LENMAX < getSize(i->RecvBuff))
		{
			cout("RECV TOO-LONG\n");
			ab_swap(buff, i->RecvBuff);
			i->HeaderSend = 1;
		}
		memFree(header);
		return;
	}
	*p = '\0';
	p = ChangeHeader(header);

	if (p)
	{
		memFree(header);
		header = p;
	}
	ab_addLine(buff, header);
	ab_addFollowBytes(buff, i->RecvBuff, GetIndexOf(i->RecvBuff, '\r'));

	i->HeaderSend = 1;
	setSize(i->RecvBuff, 0);
	memFree(header);
}
static void Perform(int sock, int fwdSock)
{
	ChInfo_t *i = nb_(ChInfo_t);

	i->RecvBuff = newBlock();

	CrossChannel(sock, fwdSock, HttpNamedChannelFltr, (uint)i, NULL, 0);

	releaseAutoBlock(i->RecvBuff);
	memFree(i);
}
static int ReadArgs(void)
{
	return 0;
}
int main(int argc, char **argv)
{
	TunnelMain(ReadArgs, Perform, "namedTrack-HTTP", NULL);
}
