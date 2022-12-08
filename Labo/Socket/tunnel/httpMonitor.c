/*
	httpMonitor.exe RECV-PORT FWD-HOST FWD-PORT /S

		/S ... 停止する。

	httpMonitor.exe RECV-PORT FWD-HOST FWD-PORT [/C CONNECT-MAX] [/SX SIZE-MAX] OUT-DIR

		CONNECT-MAX ... 最大接続数, 省略時は 1000
		SIZE-MAX ... リクエスト・レスポンスのサイズの上限, 単位はバイト, 省略時は 270mb
		OUT-DIR ... 出力先ディレクトリ

	出力先ディレクトリは、存在するディレクトリであること。

	----

	出力ファイル名

		T～T-SSSS-D-YYYYMMDDhhmmssNNN.txt

		T～T -> 接続番号
		SSSS -> 同じ接続内での番号
		D    -> 方向 0=上り, 1=下り
		NNN  -> 同じ日時での番号
*/

#include "libs\Tunnel.h"

static char *GetNextStamp(void)
{
	static char *stamp;
	static uint counter;

	if (stamp)
	{
		char *tmp = makeCompactStamp(NULL);
		int cmp;

		cmp = strcmp(stamp, tmp);

		if (cmp < 0) // ? stamp < tmp
		{
			memFree(stamp);
			stamp = tmp;
			counter = 0;
		}
		else
		{
			if (0 < cmp)
				cout("★★★システム時刻が過去方向に修正されました★★★\n");

			memFree(tmp);
			counter = m_min(counter + 1, 999);
		}
	}
	else
	{
		stamp = makeCompactStamp(NULL);
	}
	return xcout("%s%03u", stamp, counter);
}
static uint GetHeaderEndPos(autoBlock_t *buff, uint startPos)
{
	uint index;

	for (index = startPos; index + 4 <= getSize(buff); index++)
		if (!memcmp((uchar *)directGetBuffer(buff) + index, "\r\n\r\n", 4))
			return index + 4;

	return 0; // not found
}

static autoBlock_t *RCB_Block;
static uint RCB_RPos;

static char *RCB_ReadLineTo(int endChr)
{
	autoBlock_t *buff = newBlock();

	while (RCB_RPos < getSize(RCB_Block))
	{
		int chr = getByte(RCB_Block, RCB_RPos);

		RCB_RPos++;

		if (chr == endChr)
			return unbindBlock2Line(buff);

		addByte(buff, chr);
	}
	releaseAutoBlock(buff);
	return NULL;
}
static uint ReadChunkedBody(autoBlock_t *buff, uint startPos, autoBlock_t *message)
{
	RCB_Block = buff;
	RCB_RPos = startPos;

	for (; ; )
	{
		char *line = RCB_ReadLineTo('\n');
		char *p;
		uint partSize;
//		autoBlock_t gab;

		if (!line)
			break;

		p = strchr(line, ';');

		if (p)
			*p = '\0'; // chunk-extension 破棄

		partSize = toValueDigits_xc(line, hexadecimal);

		if (!partSize)
			return RCB_RPos - 2;

		if (getSize(buff) < RCB_RPos + partSize + 2)
			break;

//		addBytes(message, gndSubBytesVar(buff, RCB_RPos, partSize, gab));
		RCB_RPos += partSize + 2;
	}
	return 0; // 読み込み未完了
}
static uint ReadBody(autoBlock_t *buff, uint startPos, uint size, autoBlock_t *message)
{
	if (startPos + size <= getSize(buff))
	{
		/*
		autoBlock_t gab;

		addBytes(message, gndSubBytesVar(buff, startPos, size, gab));
		*/
		return startPos + size;
	}
	return 0; // 読み込み未完了
}

static int ChunkedFlag;
static uint ContentLength;
//static autoBlock_t *HeaderBuff;

static void ParseHeader(autoBlock_t *header)
{
	autoList_t *lines = tokenize_x(
		replaceLine(
			unbindBlock2Line(copyAutoBlock(header)),
			"\r\n",
			"\n",
			0
			),
		'\n'
		);
	char *firstLine;
	char *line;
	uint index;
	autoList_t *names = newList();
	autoList_t *values = newList();
	char *name;
	char *value;

	firstLine = strx(getLine(lines, 0));

	foreach (lines, line, index)
	{
		if (!index)
			continue;

		if (line[0] <= ' ')
		{
			if (getCount(values))
			{
				ucTrimEdge(line);

				value = (char *)getLastElement(values);
				value = addLine(value, line);
				setElement(values, getCount(values) - 1, (uint)value);
			}
		}
		else
		{
			char *p = strchr(line, ':');

			if (p)
			{
				*p = '\0';
				name = strx(line);
				value = strx(p + 1);

				ucTrimEdge(name);
				ucTrimEdge(value);

				addElement(names, (uint)name);
				addElement(values, (uint)value);
			}
		}
	}
	releaseDim(lines, 1);

	ChunkedFlag = 0;
	ContentLength = 0;
	/*
	HeaderBuff = newBlock();

	ab_addLine(HeaderBuff, firstLine);
	ab_addLine(HeaderBuff, "\r\n");
	*/

	foreach (names, name, index)
	{
		value = getLine(values, index);

		if (!_stricmp(name, "Content-Length"))
		{
			ContentLength = toValue(value);
		}
		else if (!_stricmp(name, "Transfer-Encoding"))
		{
			ChunkedFlag = mbs_stristr(value, "chunked") ? 1 : 0;
		}

		/*
		ab_addLine(HeaderBuff, name);
		ab_addLine(HeaderBuff, ": ");
		ab_addLine(HeaderBuff, value);
		ab_addLine(HeaderBuff, "\r\n");
		*/
	}
//	ab_addLine(HeaderBuff, "\r\n");

	memFree(firstLine);
	releaseDim(names, 1);
	releaseDim(values, 1);
}
static uint GetHeaderAndBody(autoBlock_t *buff, autoBlock_t *message) // ret: 0 == 読み込み未完了
{
	uint endPos = GetHeaderEndPos(buff, 0);

	if (endPos)
	{
		autoBlock_t gab;

		gndSubBytesVar(buff, 0, endPos, gab);

		ParseHeader(&gab);

//		addBytes(message, &gab);

//		addBytes(message, HeaderBuff);
//		releaseAutoBlock(HeaderBuff);

		if (ChunkedFlag)
		{
			endPos = ReadChunkedBody(buff, endPos, message);

			if (endPos)
				endPos = GetHeaderEndPos(buff, endPos); // trailer 破棄
		}
		else
		{
			endPos = ReadBody(buff, endPos, ContentLength, message);
		}
	}
	return endPos;
}

typedef struct Info_st
{
	uint64 ThID;
	uint Direction;
	autoBlock_t *Buff;
	uint64 SerialNo;
}
Info_t;

static Info_t *CreateInfo(uint64 thID, uint direction)
{
	Info_t *i = (Info_t *)memAlloc(sizeof(Info_t));

	i->ThID = thID;
	i->Direction = direction;
	i->Buff = newBlock();
	i->SerialNo = 0;

	return i;
}
static void ReleaseInfo(Info_t *i)
{
	if (getSize(i->Buff))
		cout("★★★未送信のデータを破棄します★★★\n");

	releaseAutoBlock(i->Buff);
	memFree(i);
}

static uint BuffSizeMax = 270000000;
static char *OutputDir;

static void Monitor(autoBlock_t *buff, uint prm)
{
	Info_t *i = (Info_t *)prm;
	uint endPos;
//	autoBlock_t *message = newBlock();

	addBytes(i->Buff, buff);
	setSize(buff, 0);

//	endPos = GetHeaderAndBody(i->Buff, message);
	endPos = GetHeaderAndBody(i->Buff, NULL);

	if (endPos)
	{
		char *file;
		char *stamp = GetNextStamp();

		file = combine_cx(OutputDir, xcout(
			"%I64u-%04I64u-%u-%s.txt"
			,i->ThID
			,i->SerialNo
			,i->Direction
			,stamp
			));

		i->SerialNo++;

		{
			autoBlock_t *rawMessage = desertBytes(i->Buff, 0, endPos);

			cout("%s < %u bytes\n", file, getSize(rawMessage));
			writeBinary(file, rawMessage);

			ab_addBytes_x(buff, rawMessage);
		}

		memFree(file);
		memFree(stamp);
	}
//	releaseAutoBlock(message);

	if (BuffSizeMax < getSize(i->Buff))
	{
		cout("★★★サイズオーバー！切断します★★★\n");
		ChannelDeadFlag = 1;
	}
}
static void Perform(int sock, int fwdSock)
{
	uint64 thID = nextCommonCount();
	Info_t *i;
	Info_t *j;

	i = CreateInfo(thID, 0); // 上り
	j = CreateInfo(thID, 1); // 下り

	cout("httpMonitor_接続\n");

	CrossChannel(sock, fwdSock, Monitor, (uint)i, Monitor, (uint)j);

	cout("httpMonitor_切断\n");

	ReleaseInfo(i);
	ReleaseInfo(j);
}
static int ReadArgs(void)
{
	if (argIs("/SX"))
	{
		BuffSizeMax = toValue(nextArg());
		return 1;
	}

	errorCase(!m_isRange(BuffSizeMax, 1, UINTMAX));

	OutputDir = nextArg();
	OutputDir = makeFullPath(OutputDir);

	errorCase(!existDir(OutputDir));

	return 0;
}
int main(int argc, char **argv)
{
	TunnelMain(ReadArgs, Perform, "httpMonitor", NULL);

	coSleep(500); // エラーとか見えるように...
}
