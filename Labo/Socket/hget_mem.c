#include "C:\Factory\Labo\Socket\libs\http\Client.h"
#include "C:\Factory\Common\Options\Progress.h"

#define RETCONTENTSIZEMAX (1024 * 1024 * (1024 + 64)) // 1G + 64M

int main(int argc, char **argv)
{
	char *serverDomain = NULL;
	uint portno = 80;
	char *proxyServerDomain = NULL;
	uint proxyPortno = 80;
	char *path = NULL;
	uint timeout = 0;
	autoBlock_t *content = NULL;
	autoBlock_t *retContent = NULL;
	char *outFile = NULL;
	uint retrycnt = 2;

readArgs:
	if (argIs("/S")) // Server domain
	{
		serverDomain = nextArg();
		goto readArgs;
	}
	if (argIs("/P")) // server Port-no
	{
		portno = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/PS")) // Proxy Server domain
	{
		proxyServerDomain = nextArg();
		goto readArgs;
	}
	if (argIs("/PP")) // Proxy server Port-no
	{
		proxyPortno = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/R")) // Request path (and query)
	{
		path = nextArg();
		goto readArgs;
	}
	if (argIs("/C")) // request Content
	{
		content = readBinary(nextArg());
		goto readArgs;
	}
	if (argIs("/T")) // Timeout
	{
		timeout = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/O")) // Output file
	{
		outFile = nextArg();
		goto readArgs;
	}
	if (argIs("/RET")) // RETry (0でリトライ無し)
	{
		retrycnt = toValue(nextArg());
		goto readArgs;
	}

	if (hasArgs(1))
	{
		char *p;

		serverDomain = strx(nextArg());

		if (p = strchr(serverDomain, '/'))
		{
			path = strx(p);
			*p = '\0';
		}
		if (p = strchr(serverDomain, ':'))
		{
			portno = toValue(p + 1);
			*p = '\0';
		}
	}
	if (hasArgs(1))
		outFile = nextArg();

	if (!path)
		path = "/";

	if (outFile)
		remove(outFile);

retry:
/*
	ProgressBegin();
	SockSendInterlude = Progress;
	SockRecvInterlude = Progress;
*/
	retContent = httpSendRequest(serverDomain, portno, proxyServerDomain, proxyPortno, path, content, timeout, RETCONTENTSIZEMAX);
/*
	SockSendInterlude = NULL;
	SockRecvInterlude = NULL;
	ProgressEnd(retContent ? 0 : 1);
*/

	if (retContent)
	{
		if (!outFile)
		{
			char *outStr = toPrintLine(retContent, 1);
			cout("%s\n", outStr);
			memFree(outStr);
		}
		else
		{
			writeBinary(outFile, retContent);
		}
		releaseAutoBlock(retContent);
	}
	else
	{
		cout("失敗しました。\n");

		if (retrycnt)
		{
			cout("あと %u 回リトライします。\n", retrycnt);
			retrycnt--;
			goto retry;
		}
	}

	if (content)
		releaseAutoBlock(content);
}
