/*
	Get/Post request and response for HTTP
*/

#include "GetPost.h"

#define HEADERLENMAX (1024 * 64)
#define HEADERTOTALLENMAX (1024 * 512)
#define DEF_CONTENTSIZEMAX (1024 * 1024 * 4)
#define HOSTLENMAX 300

/*
	NULL �łȂ���΁u�ŏ��̍s���܂ށv�S�Ẵw�b�_�s�� addElement(, hdr_line) ����B�e�s�� lineToPrintLine() ����Ă���B
	����͌Ăяo������ releaseDim(, 1); �������s�����ƁB
	�ꎞ�I�Ȏg�p�ł���Ȃ�� NULL �ɖ߂����Ƃ�Y�ꂸ�ɁB
*/
autoList_t *httpRecvedHeader;

int httpRecvRequestFirstWaitDisable;
uint httpRecvRequestFirstWaitMillis = 2000;
char *httpRecvRequestHostValue;

void httpRecvRequestHeader(SockStream_t *i, char **pHeader, int *pChunked, uint *pContentSize)
{
	uint i_rslmt = i->Extra.RecvSizeLimiter;
	int chunked = 0;
	uint cSize = 0;
	char *reqHost = NULL;
	char *line;
	char *header;
	char *backedLine = NULL;

	/*
		IE9 �͂��炩���ߐڑ����Ă����āA���N�G�X�g���������Ă���f�[�^�𑗂���ĂȂ��Ƃ�����B
		���̐ڑ��́A�q���ł���̂ɉ������Ȃ��ڑ��Ƃ��ċ�����B�ז��Ȃ̂ŁA�Ƃ��ƂƐ؂�B
		��莞�ԒʐM�������ڑ��ɂ��āA�f�[�^�������瑦�ؒf����Ƃ����̂������Ă݂����A
		IE9 �͍Đڑ����ăy�[�W���擾�A���������������悤�Ƀy�[�W��\�������̂ŁA�����l�����Ԃ����؂��Ă���薳�����낤�B
	*/
	if (!httpRecvRequestFirstWaitDisable &&
		SockWait(GetSockStreamSock(i), httpRecvRequestFirstWaitMillis, 0) != 1)
	{
		DestroySockStream(i);
	}

	i->Extra.RecvSizeLimiter = HEADERTOTALLENMAX;
	header = SockRecvLine(i, HEADERLENMAX);

	if (httpRecvedHeader)
		addElement(httpRecvedHeader, (uint)lineToPrintLine(header, 0));

	for (; ; )
	{
		if (backedLine)
			line = backedLine;
		else
			line = SockRecvLine(i, HEADERLENMAX);

		if (!*line)
		{
			memFree(line);
			break;
		}
		for (; ; )
		{
			char *trail = SockRecvLine(i, HEADERLENMAX);
			char *p;

			if (!*trail || ' ' < *trail)
			{
				backedLine = trail;
				break;
			}
			for (p = trail; *p; p++)
				if (' ' < *p)
					break;

			line = addChar(line, ' ');
			line = addLine(line, p);

			memFree(trail);
		}

		if (lineExpICase("<>Transfer-Encoding<>:<>chunked<>", line))
		{
			chunked = 1;
		}
		else if (lineExpICase("<>Content-Length<>:<><09><>", line))
		{
			char *p = strchrNext(line, ':');

			// ���Ȃ��Ƃ� IE8 �ł� 2GB �𒴂���� -((2 p 32) - cSize) / 10 �Ƃ����ςȒl�ɂȂ�B

			if (strchr(p, '-')) // ? �}�C�i�X
			{
				cSize = 0;
			}
			else
			{
				cSize = toValue(p);
			}
		}
		else if (lineExpICase("<>Host<>:<>", line))
		{
			char *p = strchrNext(line, ':');

			strchrEnd(p, ':')[0] = '\0'; // �|�[�g�ԍ�������
			ucTrimEdge(p);

			if (HOSTLENMAX < strlen(p))
				p[HOSTLENMAX] = '\0';

			reqHost = strz(reqHost, p);
		}

		if (httpRecvedHeader)
			addElement(httpRecvedHeader, (uint)lineToPrintLine(line, 0));

		memFree(line);
	}
	if (httpRecvRequestHostValue)
	{
		{
			char *p = reqHost;

			if (p)
			{
				p = strx(p);
				line2JLine(p, 0, 0, 0, 0);
			}
			else
			{
				p = strx("<NULL>");
			}

			cout("C-Host: %s\n", httpRecvRequestHostValue);
			cout("R-Host: %s\n", p);

			memFree(p);
		}

		if (!reqHost || _stricmp(reqHost, httpRecvRequestHostValue))
		{
			DestroySockStream(i);
		}
	}
	memFree(reqHost);

	*pHeader = header;
	*pChunked = chunked;
	*pContentSize = cSize;

	i->Extra.RecvSizeLimiter = i_rslmt;
}
void httpRecvRequest(SockStream_t *i, char **pHeader, uchar **pContent, uint *pContentSize)
{
	httpRecvRequestMax(i, pHeader, pContent, pContentSize, DEF_CONTENTSIZEMAX);
}
void httpRecvRequestMax(SockStream_t *i, char **pHeader, uchar **pContent, uint *pContentSize, uint contentSizeMax)
{
	char *header;
	int chunked;
	uint cSize;
	uint nextCSize;
	uchar *content;
	char *line;

	httpRecvRequestHeader(i, &header, &chunked, &cSize);

	if (chunked)
	{
		httpChunkedRecver_t *cr = httpCreateChunkedRecver(i);

		content = (uchar *)memAlloc(1); // �Ō�� '\0' �ŕ���̂� +1
		cSize = 0;

		for (; ; )
		{
			autoBlock_t *block = httpRecvChunked(cr);

			if (!block)
			{
				break;
			}
			if (contentSizeMax - cSize < getSize(block)) // Overflow -> �����ɓǂݍ��ݒ��f
			{
				releaseAutoBlock(block);
				break;
			}
			content = (uchar *)memRealloc(content, cSize + getSize(block) + 1);
			memcpy(content + cSize, directGetBuffer(block), getSize(block));
			cSize += getSize(block);

			releaseAutoBlock(block);
		}
		httpReleaseChunkedRecver(cr);
	}
	else
	{
		cSize = m_min(cSize, contentSizeMax); // Overflow -> ����ɋ���

		content = (uchar *)memAlloc(cSize + 1);
		SockRecvBlock(i, content, cSize);
	}
	content[cSize] = '\0'; // ������Ƃ��Ă��������B

	*pHeader = header;
	*pContent = content;
	*pContentSize = cSize;
}
void httpSendResponseHeader(SockStream_t *i, uint64 cSize, char *contentType)
{
	char *sCSize = xcout("%I64u", cSize);

	SockSendLine(i, "HTTP/1.1 200 OK");
	SockSendToken(i, "Content-Type: ");
	SockSendLine(i, contentType);
	SockSendToken(i, "Content-Length: ");
	SockSendLine(i, sCSize);

	if (i->Extra.SaveFile)
	{
		SockSendToken(i, "Content-Disposition: attachment; filename=\"");
		SockSendToken(i, i->Extra.SaveFile);
		SockSendLine(i, "\"");
	}
	if (i->Extra.ServerName)
	{
		SockSendToken(i, "Server: ");
		SockSendLine(i, i->Extra.ServerName);
	}
	SockSendLine(i, "");

	memFree(sCSize);
}
void httpSendResponse(SockStream_t *i, autoBlock_t *content, char *contentType)
{
	httpSendResponseHeader(i, getSize(content), contentType);
	SockSendBlock(i, directGetBuffer(content), getSize(content));
	SockFlush(i);
}
void httpSendResponseText(SockStream_t *i, char *text, char *contentType)
{
	autoBlock_t gab;
	httpSendResponse(i, gndBlockLineVar(text, gab), contentType);
}

/*
	�����R�[�h�� Shift_JIS (US-ASCII) ��z�肷��B

	���N�G�X�g�̕����R�[�h�� Shift_JIS �ɂ���ɂ͈ȉ��� html �ɋL�q����B
		<meta http-equiv="Content-Type" content="text/html; charset=Shift_JIS"/> (*1)
		<form method="????" accept-charset="Shift_JIS" action="????">

		(*1) �ł��邾����̕��ɋL�q����B
		     ���X�|���X�w�b�_�� Content-Type: text/html; charset=Shift_JIS ���L�q���Ă��ǂ��B
*/
void httpUrlDecoder(char *line)
{
	char *p;

	for (p = line; *p; p++)
	{
		if (*p == '%')
		{
			if (m_ishexadecimal(p[1]) && m_ishexadecimal(p[2]))
			{
				*p = c2i(p[1]) << 4 | c2i(p[2]);
				copyLine(p + 1, p + 3);
			}
		}
		else if (*p == '+')
		{
			*p = ' ';
		}
	}
}
char *httpUrlEncoder(char *line)
{
	char *encLine = strx("");
	char *p;

	for (p = line; *p; p++)
	{
		encLine = addLine_x(encLine, xcout("%%%02x", *p));
	}
	return encLine;
}
void httpDecodeUrl(char *url, httpDecode_t *out)
{
	char *path = strstr(url, "://");
	autoList_t *dirList;
	char *dir;
	uint index;
	char *decPath;

	if (path)
		path = strchrEnd(path + 3, '/');
	else
		path = url;

	if (!*path)
		path = "/";

	path = strx(path);

	dirList = tokenize(path, '/');
	trimLines(dirList);

	foreach (dirList, dir, index)
		httpUrlDecoder(dir);

	decPath = untokenize(dirList, "/");

	out->Path = path;
	out->DecPath = decPath;
	out->DirList = dirList;
}
void httpDecodeQuery(char *query, httpDecode_t *out)
{
	autoList_t *keys = newList();
	autoList_t *values = newList();
	autoList_t *parts;
	char *part;
	uint index;

	parts = tokenize(query, '&');

	foreach (parts, part, index)
	{
		char *p = strchr(part, '=');

		if (p)
		{
			char *key;
			char *value;

			*p = '\0';
			key = strx(part);
			value = strx(p + 1);

			httpUrlDecoder(key);
			httpUrlDecoder(value);

			addElement(keys, (uint)key);
			addElement(values, (uint)value);
		}
	}
	releaseDim(parts, 1);

	out->Keys = keys;
	out->Values = values;
}
void httpDecode(char *header, uchar *content, httpDecode_t *out)
{
	char *url;
	char *p;

	url = strchrNext(header, ' ');
	p = strchrEnd(url, ' ');

	url = strxl(url, (uint)p - (uint)url);

	if (m_toupper(header[0]) == 'G') // ? 'Get' Request
	{
		p = strchr(url, '?');

		if (p)
		{
			*p = '\0';
			p++;
		}
		else
		{
			p = "";
		}
		content = (uchar *)p;
	}
	out->Url = url;
	out->Query = strx((char *)content);

	httpDecodeUrl(out->Url, out);
	httpDecodeQuery(out->Query, out);
}
void httpDecodeFree(httpDecode_t *out)
{
	// httpDecode()
	memFree(out->Url);
	memFree(out->Query);

	// httpDecodeUrl()
	memFree(out->Path);
	memFree(out->DecPath);
	releaseDim(out->DirList, 1);

	// httpDecodeQuery()
	releaseDim(out->Keys, 1);
	releaseDim(out->Values, 1);
}

void httpDecodeHeader(char *header, httpDecode_t *out)
{
	autoList_t *tokens = tokenize(header, ' ');
	char *url;

	url = refLine(tokens, 1);
	httpDecodeUrl(url, out);
	releaseDim(tokens, 1);
}
void httpDecodeHeaderFree(httpDecode_t *out)
{
	memFree(out->Path);
	memFree(out->DecPath);
	releaseDim(out->DirList, 1);
}

void httpSendResponseFileType(SockStream_t *i, char *file, char *contentType)
{
//	uint fileIOMtx = mutexOpen(HTTPSENDRESPONSEFILETYPE_FILEIO_MUTEXNAME);
	uint64 cSize;
	uint64 sendcnt;

//	handleWaitForever(fileIOMtx);
	{
		cSize = getFileSize(file);
	}
//	mutexRelease(fileIOMtx);

	httpSendResponseHeader(i, cSize, contentType);
	sendcnt = 0;

#define SEND_BUFF_SIZE (1024 * 512)

	while (sendcnt < cSize)
	{
		autoBlock_t *buffBlock = nobCreateBlock((uint)m_min(cSize - sendcnt, (uint64)SEND_BUFF_SIZE));

//		handleWaitForever(fileIOMtx);
		{
			FILE *fp = fileOpen(file, "rb");
			fileSeek(fp, SEEK_SET, sendcnt);
			fileRead(fp, buffBlock);
			fileClose(fp);
		}
//		mutexRelease(fileIOMtx);

		SockSendBlock(i, directGetBuffer(buffBlock), getSize(buffBlock));

		sendcnt += getSize(buffBlock);
		releaseAutoBlock(buffBlock);
	}
	SockFlush(i);

//	handleClose(fileIOMtx);
}
void httpSendResponseFile(SockStream_t *i, char *file)
{
	httpSendResponseFileType(i, file, httpExtToContentType(getExt(file)));
}
