/*
	H_Request, H_Keys, H_Values は整形済み (line2JLine etc.) の文字列をセットする。
*/

#include "HTTPParse.h"

HttpDat_t HttpDat;

static void ClearHttpDat(void)
{
	if (HttpDat.Header)
		releaseDim(HttpDat.Header, 1);

	memFree(HttpDat.H_Request);

	if (HttpDat.H_Keys)
		releaseDim(HttpDat.H_Keys, 1);

	if (HttpDat.H_Values)
		releaseDim(HttpDat.H_Values, 1);

	if (HttpDat.Body)
		releaseAutoBlock(HttpDat.Body);

	memset(&HttpDat, 0x00, sizeof(HttpDat_t));
}

HttpDat_t EjectHttpDat(void)
{
	HttpDat_t backup = HttpDat;

	memset(&HttpDat, 0x00, sizeof(HttpDat_t));
	return backup;
}
void UnejectHttpDat(HttpDat_t backup)
{
	ClearHttpDat();
	HttpDat = backup;
}

static autoBlock_t *RBuff;
static uint RPos;

static char *ReadLine(void)
{
	autoBlock_t *buff = newBlock();

	while (RPos < getSize(RBuff))
	{
		int chr = getByte(RBuff, RPos++);

		if (chr == '\r')
			continue;

		if (chr == '\n')
			return unbindBlock2Line(buff);

		addByte(buff, chr);
	}
	releaseAutoBlock(buff);
	return NULL;
}
static int SkipTrailer(void)
{
	char *line;

	while (line = ReadLine())
	{
		if (*line == '\0')
		{
			memFree(line);
			return 1;
		}
		memFree(line);
	}
	return 0;
}

static int ReadHeader(void)
{
	char *line;

	HttpDat.Header = newList();

	while (line = ReadLine())
	{
		if (*line == '\0')
		{
			memFree(line);
			return 1;
		}
		addElement(HttpDat.Header, (uint)line);
	}
	return 0;
}
static void HeaderParse(void)
{
	char *line;
	uint index;

	HttpDat.H_Request = NULL; // 2bs: たぶん不要
	HttpDat.H_Keys = newList();
	HttpDat.H_Values = newList();

	foreach (HttpDat.Header, line, index)
	{
		line = strx(line);
		line2JLine(line, 1, 0, 1, 1);
		replaceChar(line, '\t', ' ');

		if (index == 0)
		{
			trimEdge(line, ' ');

			HttpDat.H_Request = strx(line);
		}
		else if (line[0] == ' ') // -> unfolding
		{
			trimEdge(line, ' ');

			if (getCount(HttpDat.H_Values))
			{
				char *value = (char *)getLastElement(HttpDat.H_Values);

				value = addChar(value, ' '); // HACK
				value = addLine(value, line);
				setElement(HttpDat.H_Values, getCount(HttpDat.H_Values) - 1, (uint)value);
			}
		}
		else
		{
			char *p = strchr(line, ':');

			if (p)
			{
				char *key;
				char *value;

				key = line;
				*p = '\0';
				p++;
				value = p;

				key = strx(key);
				value = strx(value);

				trimEdge(key, ' ');
				trimEdge(value, ' ');

				addElement(HttpDat.H_Keys, (uint)key);
				addElement(HttpDat.H_Values, (uint)value);
			}
		}
		memFree(line);
	}
	if (!HttpDat.H_Request)
		HttpDat.H_Request = strx(""); // dummy
}
static void CheckHeader(void)
{
	char *key;
	uint index;

	foreach (HttpDat.H_Keys, key, index)
	{
		char *value = getLine(HttpDat.H_Values, index);

		if (!_stricmp(key, "transfer-encoding"))
		{
			HttpDat.Chunked = (int)mbs_stristr(value, "chunked");
		}
		else if (!_stricmp(key, "content-length"))
		{
			HttpDat.ContentLength = toValue(value);
		}
		else if (!_stricmp(key, "expect"))
		{
			HttpDat.Expect100Continue = (int)mbs_stristr(value, "100-continue");
cout("HttpDat.Expect100Continue: %d\n", HttpDat.Expect100Continue);
		}
	}
}
static int ReadBody(void)
{
	if (HttpDat.Expect100Continue)
	{
		HttpDat.Body = newBlock();
LOGPOS();
		return 1;
	}
	if (HttpDat.Chunked)
	{
		HttpDat.Body = newBlock();

		for (; ; )
		{
			char *line = ReadLine();
			char *p;
			uint partSize;
			autoBlock_t gab;

			if (!line)
				break;

			p = strchr(line, ';');

			if (p)
				*p = '\0'; // chunk-extension 破棄

			partSize = toValueDigits_xc(line, hexadecimal);

			if (!partSize) // ? 全パート読み込み完了
			{
				if (!SkipTrailer()) // ? trailer 未読
					break;

				return 1;
			}
			if (getSize(RBuff) - RPos < partSize) // ? パート未読
				break;

			addBytes(HttpDat.Body, gndSubBytesVar(RBuff, RPos, partSize, gab));
			RPos += partSize;

			if (getSize(RBuff) - RPos < 2) // ? パートの後の CR-LF 未読
				break;

			RPos += 2; // CR-LF
		}
	}
	else
	{
		if (HttpDat.ContentLength <= getSize(RBuff) - RPos)
		{
			autoBlock_t gab;

			HttpDat.Body = copyAutoBlock(gndSubBytesVar(RBuff, RPos, HttpDat.ContentLength, gab));
			RPos += HttpDat.ContentLength;
			return 1;
		}
	}
	return 0;
}

int HTTPParse(autoBlock_t *buff)
{
	ClearHttpDat();

	RBuff = buff;
	RPos = 0;

	if (!ReadHeader())
		return 0;

	HeaderParse();
	CheckHeader();

	if (!ReadBody())
		return 0;

	HttpDat.EndPos = RPos;
	return 1;
}

void SaveHttpDat(char *file)
{
	FILE *fp = fileOpen(file, "wb");
	char *key;
	uint index;

	// 2bs
	errorCase(!HttpDat.H_Request);
	errorCase(!HttpDat.H_Keys);
	errorCase(!HttpDat.H_Values);
	errorCase(!HttpDat.Body);

	writeToken(fp, HttpDat.H_Request);
	writeToken(fp, "\r\n");

	foreach (HttpDat.H_Keys, key, index)
	{
		writeToken(fp, key);
		writeToken(fp, ": ");
		writeToken(fp, getLine(HttpDat.H_Values, index));
		writeToken(fp, "\r\n");
	}
	writeToken(fp, "\r\n");
	writeBinaryBlock(fp, HttpDat.Body);

	fileClose(fp);
}
static autoBlock_t *ReadToEnd(FILE *fp)
{
	autoBlock_t *buff = newBlock();

	for (; ; )
	{
		int chr = readChar(fp);

		if (chr == EOF)
			break;

		addByte(buff, chr);
	}
	return buff;
}
void LoadHttpDat(char *file)
{
	FILE *fp = fileOpen(file, "rb");

	ClearHttpDat();

	HttpDat.Header = newList();

	for (; ; )
	{
		char *line = readLine(fp);

		errorCase(!line);

		if (!*line)
			break;

		addElement(HttpDat.Header, (uint)line);
	}
	HttpDat.Body = ReadToEnd(fp);

	fileClose(fp);

	HeaderParse();
	CheckHeader();

	HttpDat.EndPos = IMAX; // てきとう
}
