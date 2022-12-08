#include "httpClient.h"

uint httpGetOrPostRetryMax = 2;
uint httpGetOrPostRetryDelayMillis = 2000;
uint httpGetOrPostTimeoutSec = 180;
uint64 httpGetOrPostRecvBodySizeMax = 20000000ui64; // 20 MB
char *httpGetOrPostProxyDomain = NULL;
uint httpGetOrPostProxyPortNo = 8080;

/*
	recvBodyFile ... �������ɕK���쐬����B���s���͕K���폜����B
*/
int httpGetOrPostFile(char *url, autoBlock_t *sendBody, char *recvBodyFile) // sendBody: NULL == GET, ret: ? ����
{
	char *domain;
	char *p;
	char *path;
	uint portNo;
	uint retry;
	int ret;

	errorCase(!url);
//	sendBody
	errorCase(m_isEmpty(recvBodyFile));

	if (!*url)
		return 0;

	domain = strx(url);

	if (startsWith(domain, "http://"))
		eraseLine(domain, 7);

	if (p = strchr(domain, '/'))
	{
		path = strx(p);
		*p = '\0';
	}
	else
	{
		path = strx("/");
	}

	if (p = strchr(domain, ':'))
	{
		portNo = toValue(p + 1);
		*p = '\0';
	}
	else
	{
		portNo = 80;
	}

	httpMultiPartContentLenMax = httpGetOrPostRecvBodySizeMax;

	/*
		sendBody != NULL �̏ꍇ�A
		httpExtraHeader �� Content-Type ���w�肵�Ă����Ȃ��ƁAClient.c �ŏ���ɕt������B
	*/

	SockStartup();

	for (retry = 0; retry <= httpGetOrPostRetryMax; retry++)
	{
		ret = httpSendRequestFile(
			domain,
			portNo,
			httpGetOrPostProxyDomain,
			httpGetOrPostProxyPortNo,
			path,
			sendBody,
			httpGetOrPostTimeoutSec,
			recvBodyFile
			);

		if (ret)
			break;

		coSleep(httpGetOrPostRetryDelayMillis);
	}
	SockCleanup();

	memFree(domain);
	memFree(path);

	return ret;
}
autoBlock_t *httpGetOrPost(char *url, autoBlock_t *sendBody) // sendBody: NULL == GET, ret: NULL == ���s
{
	char *recvBodyFile;
	autoBlock_t *recvBody;

	errorCase(!url);
//	sendBody

	recvBodyFile = makeTempPath(NULL);

	if (httpGetOrPostFile(url, sendBody, recvBodyFile))
	{
		recvBody = readBinary(recvBodyFile);
		removeFile(recvBodyFile);
	}
	else
	{
		recvBody = NULL;
	}
	memFree(recvBodyFile);
	return recvBody;
}
