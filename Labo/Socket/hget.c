/*
	�t�@�C���̃_�E�����[�h
		hget.exe http://www.cs.ucdavis.edu/~rogaway/ocb/ocb-ref/rijndael-alg-fst.c

	�_�E�����[�h���Ďw��t�@�C���ɕۑ�����B
		hget.exe /o 123.c http://www.cs.ucdavis.edu/~rogaway/ocb/ocb-ref/rijndael-alg-fst.c
		hget.exe http://www.cs.ucdavis.edu/~rogaway/ocb/ocb-ref/rijndael-alg-fst.c 123.c

	�_�E�����[�h���ĉ�ʂɕ\������B
		hget.exe /o * http://www.cs.ucdavis.edu/~rogaway/ocb/ocb-ref/rijndael-alg-fst.c
		hget.exe http://www.cs.ucdavis.edu/~rogaway/ocb/ocb-ref/rijndael-alg-fst.c *
*/

#include "C:\Factory\Labo\Socket\libs\http\Client.h"
#include "C:\Factory\Common\Options\Progress.h"
#include "C:\Factory\Common\Options\UTF.h"

#define MUTEX_ID "{c2a09038-995b-492b-ae74-f45d04823852}"

static void TextFltr(char *file, uint lineLenMax)
{
	char *midFile = makeTempPath(NULL);
	FILE *rfp;
	FILE *wfp;
	char *line;

	cout("TextFltr lineLenMax=%u\n", lineLenMax);

	rfp = fileOpen(file, "rt");
	wfp = fileOpen(midFile, "wt");

	while (line = readLineLenMax(rfp, lineLenMax))
	{
		line2JLine(line, 1, 0, 1, 1);
		writeLine_x(wfp, line);
	}
	fileClose(rfp);
	fileClose(wfp);

	removeFile(file);
	moveFile(midFile, file);
	memFree(midFile);

	cout("TextFltr ok\n");
}
static void Progress_Wrap(void)
{
	static uint callcnt;

	callcnt++;

	if (eqIntPulseSec(1, NULL))
	{
		cmdTitle_x(xcout("hget - %u (S:%I64u, R:%I64u)", callcnt, SockTotalSendSize, SockTotalRecvSize));
		Progress();
	}
}
int main(int argc, char **argv)
{
	char *serverDomain = NULL;
	uint portno = 80;
	char *proxyServerDomain = NULL;
	uint proxyPortno = 80;
	char *path = NULL;
	uint timeout = 0;
	autoBlock_t *content = NULL;
	char *outFile = NULL;
	char *resFile = makeTempFile("res");
	uint retrycnt = 2;
	int jflag = 0;
	int jflag_utf8 = 0;
	int retval;
	uint outFileTextFltrLineLenMax = 0; // 0 as disabled
	int successful = 0;
	uint lastStartTime;
	int outputAndOpenOutDir = 0;
	char *hdrOutFile = NULL;
	int mtxFlag = 0;
	uint mtx;

	httpM4UServerMode = 1;
	timeout = 86400;
	httpBlockTimeout = 180;

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
	if (argIs("/C")) // request Content (������w�肷���POST�ɂȂ�)
	{
		content = readBinary(nextArg());
		goto readArgs;
	}
	if (argIs("/T")) // Timeout (�ڑ����Ă���̃^�C���A�E�g, �b, 0 == ������)
	{
		timeout = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/BT")) // Block Timeout (���ʐM�^�C���A�E�g, �b, 0 == ���ʐM)
	{
		httpBlockTimeout = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/O")) // Output file
	{
		outFile = nextArg();
		goto readArgs;
	}
	if (argIs("/RET")) // RETry (0�Ń��g���C����)
	{
		retrycnt = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/J")) // ��M�f�[�^��\������Ƃ����{����ǂ߂�悤�ɂ���B�A�� SJIS ����Ȃ��Ƒʖڂ��� JIS0208.txt ���K�v�A����܈Ӗ��Ȃ������ˁB
	{
		jflag = 1;
		goto readArgs;
	}
	if (argIs("/J8")) // �\���f�[�^��\������Butf-8
	{
		jflag = 1;
		jflag_utf8 = 1;
		goto readArgs;
	}
	if (argIs("/RSX")) // Recv content Size maX
	{
		httpMultiPartContentLenMax = toValue(nextArg()); // �f�t�H���g�� 2G ���炢�B
		goto readArgs;
	}
	if (argIs("/RTX")) // Recv content (output file) Text filter line-length maX
	{
		outFileTextFltrLineLenMax = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/H")) // �g���w�b�_�t�B�[���h
	{
		httpExtraHeader = readLines(nextArg());
		goto readArgs;
	}
	if (argIs("/-M4U")) // m4u�T�[�o�[���[�hoff
	{
		httpM4UServerMode = 0;
		goto readArgs;
	}
	if (argIs("/HH")) // ��M�w�b�_��\������B
	{
		httpRecvedHeader = newList();
		goto readArgs;
	}
	if (argIs("/HHO")) // ��M�w�b�_���t�@�C���ɕۑ�����B
	{
		httpRecvedHeader = newList();
		hdrOutFile = nextArg();
		goto readArgs;
	}
	if (argIs("/M")) // �r���I�Ɏ��s����B
	{
		mtxFlag = 1;
		goto readArgs;
	}
	if (argIs("/L")) // BlueFish ���̃o�b�`�p
	{
		cout("#### LITE MODE ####\n");
		sockConnectTimeoutSec = 5;
//		sockConnectTimeoutSec = 2; // ieServer�ł悭timeout����B@ 2019.3.14
		httpM4UServerMode = 0;
		timeout = 30;
		httpBlockTimeout = 20;
		retrycnt = 0;
		httpMultiPartContentLenMax = 2000000; // 2mb
		outFile = "*";
		httpRecvedHeader = newList();
	}

	if (hasArgs(1))
	{
		char *p;

		serverDomain = strx(nextArg());

		if (startsWith(serverDomain, "http://"))
			eraseLine(serverDomain, 7);

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
	{
		if (*outFile == '*')
			outFile = NULL;
	}
	else
	{
		char *p = strrchr(path, '/');

		if (p)
			p++;
		else
			p = outFile;

		outFile = getOutFile(lineToFairLocalPath(p, 50));
		outputAndOpenOutDir = 1;
	}
	if (outFile)
		remove(outFile);

retry:
	lastStartTime = now();

	if (httpRecvedHeader)
	{
		releaseDim(httpRecvedHeader, 1);
		httpRecvedHeader = newList();
	}

	if (mtxFlag)
	{
		LOGPOS();
		mtx = mutexLock(MUTEX_ID);
		LOGPOS();
	}

	LOGPOS();

	ProgressBegin();
	SockSendInterlude = Progress_Wrap;
	SockRecvInterlude = Progress_Wrap;

	retval = httpSendRequestFile(serverDomain, portno, proxyServerDomain, proxyPortno, path, content, timeout, resFile);

	SockSendInterlude = NULL;
	SockRecvInterlude = NULL;
	ProgressEnd(retval ? 0 : 1);

	LOGPOS();

	if (mtxFlag)
	{
		LOGPOS();
		mutexUnlock(mtx);
		LOGPOS();
	}

	cmdTitle("hget - done");

	if (retval)
	{
		if (httpRecvedHeader)
		{
			char *rh_line;
			uint rh_index;

			cout("����M�w�b�_��������\n");

			foreach (httpRecvedHeader, rh_line, rh_index)
				cout("%s\n", rh_line);

			cout("����M�w�b�_�����܂�\n");

			if (hdrOutFile)
				writeLines(hdrOutFile, httpRecvedHeader);

			releaseDim(httpRecvedHeader, 1);
			httpRecvedHeader = NULL;
		}
		if (outFile)
		{
			if (existFile(outFile))
				removeFile(outFile);

			moveFile(resFile, outFile);

			if (outFileTextFltrLineLenMax)
				TextFltr(outFile, outFileTextFltrLineLenMax);

			if (outputAndOpenOutDir)
				openOutDir();
		}
		else
		{
			FILE *fp = fileOpen(resFile, "rt");
			char *line;

			cout("����M�f�[�^\n");

			while (line = readLineLenMax(fp, 128 * 1024 * 1024))
			{
				char *pLine;

				if (jflag)
				{
					if (jflag_utf8)
					{
						char *tmpfile = makeTempPath("utf");

						writeOneLineNoRet_b(tmpfile, line);
						memFree(line);
						UTF8ToSJISFile(tmpfile, tmpfile);
						line = readText_b(tmpfile);

						removeFile(tmpfile);
						memFree(tmpfile);
					}
					line2JLine(pLine = strx(line), 1, 1, 0, 1);
				}
				else
				{
					pLine = lineToPrintLine(line, 0);
				}

				cout("%s\n", pLine);

				memFree(pLine);
				memFree(line);
			}
			fileClose(fp);
			removeFile(resFile);
		}
		successful = 1;
	}
	else
	{
		cout("���s���܂����B\n");

		if (retrycnt)
		{
			cout("���� %u �񃊃g���C���܂��B\n", retrycnt);
			retrycnt--;

			LOGPOS();
			while (now() <= lastStartTime + 2) sleep(300);
			LOGPOS();

			goto retry;
		}
	}
	memFree(resFile);

	return successful ? 0 : 1;
}
