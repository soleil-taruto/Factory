/*
	Spoof.exe RECV-PORT FWD-HOST FWD-PORT /S

		/S ... 停止する。

	Spoof.exe RECV-PORT FWD-HOST FWD-PORT [/C CONNECT-MAX] [/T TIMEOUT] [/SF SEND-PTNTBL-FILE] [/RF RECV-PTNTBL-FILE]

		CONNECT-MAX ... 最大接続数, 省略時は 1000
		TIMEOUT ... 置き換えパターンの途中で通信が止まったときのタイムアウト [秒], 0 == 無効, 省略時は 2 秒

		SEND-PTNTBL-PATH, RECV-PTNTBL-PATH ...

			置き換えパターンを記述したバイナリファイル名のリストを記述したファイル名を指定します。
			ディレクトリを指定した場合は、その配下のファイルをパス名でソートしたものをリストと見なします。
			リストの長さは偶数で、奇数番目が置き換え前のパターン、その直後の偶数番目が置き換え後のパターンになるようにします。
*/

#include "libs\Tunnel.h"

typedef struct FltrInfo_st
{
	char *Header;
	autoBlock_t *Buff;
	uint CloggedTimeout;
	autoList_t *SrcPtns;
	autoList_t *DestPtns;
}
FltrInfo_t;

static FltrInfo_t *SendFltrInfo;
static FltrInfo_t *RecvFltrInfo;
static uint CloggedTimeoutSec = 2;

static FltrInfo_t *GetFltrInfo(char *path, char *header)
{
	FltrInfo_t *i = nb_(FltrInfo_t);
	autoList_t *lines;
	uint index;

	i->Header = header;
	i->Buff = newBlock();
	i->CloggedTimeout = UINTMAX;
	i->SrcPtns = newList();
	i->DestPtns = newList();

	if (existDir(path))
	{
		lines = lssFiles(path);
		sortJLinesICase(lines);
	}
	else
	{
		lines = readLines(path);
	}

	for (index = 0; index < getCount(lines); index += 2)
	{
		char *file1 = getLine(lines, index);
		char *file2 = getLine(lines, index + 1);
		autoBlock_t *ptn1;
		autoBlock_t *ptn2;

		ptn1 = readBinary(file1);
		ptn2 = readBinary(file2);

		cout("< %s (%u)\n", file1, getSize(ptn1));
		cout("> %s (%u)\n", file2, getSize(ptn2));

		errorCase(!getSize(ptn1));

		addElement(i->SrcPtns, (uint)ptn1);
		addElement(i->DestPtns, (uint)ptn2);
	}
	return i;
}

static void StrmCharFltr(FltrInfo_t *i, autoBlock_t *dest, int chr)
{
	uint minPos;
	uint sPos;
	uint ptnndx;
	autoBlock_t gab;

	addByte(i->Buff, chr);
	minPos = getSize(i->Buff);

	for (sPos = getSize(i->Buff) - 1; ; sPos--)
	{
		void *p = ((uchar *)directGetBuffer(i->Buff)) + sPos;
		uint size = getSize(i->Buff) - sPos;
		autoBlock_t *srcPtn;

		foreach (i->SrcPtns, srcPtn, ptnndx)
		{
			if (size <= getSize(srcPtn))
			{
				void *q = directGetBuffer(srcPtn);

				if (!memcmp(p, q, size))
				{
					if (size == getSize(srcPtn))
						goto found;

					minPos = sPos;
				}
			}
		}
		if (!sPos)
			break;
	}
	addBytes(dest, gndSubBytesVar(i->Buff, 0, minPos, gab));
	removeBytes(i->Buff, 0, minPos);
	return;

found:
	LOGPOS();
	addBytes(dest, gndSubBytesVar(i->Buff, 0, sPos, gab));
	addBytes(dest, (autoBlock_t *)getElement(i->DestPtns, ptnndx));
	setSize(i->Buff, 0);
}
static void CheckClogging(FltrInfo_t *i, autoBlock_t *dest)
{
	if (!CloggedTimeoutSec)
		return;

	if (getSize(i->Buff) && getSize(dest) == 0) // ? 詰まっている。
	{
		if (i->CloggedTimeout < now()) // ? タイムアウト
		{
			cout("+---------------------+\n");
			cout("| CLOGGING-UP TIMEOUT |\n");
			cout("+---------------------+\n");

			ab_swap(i->Buff, dest);

			i->CloggedTimeout = UINTMAX;
		}
	}
	else
	{
		i->CloggedTimeout = now() + CloggedTimeoutSec;
	}
}
static void StrmFltr(autoBlock_t *buff, uint prm)
{
	FltrInfo_t *i = (FltrInfo_t *)prm;
	autoBlock_t *dest = newBlock();
	uint index;

	for (index = 0; index < getSize(buff); index++)
		StrmCharFltr(i, dest, getByte(buff, index));

	ab_swap(buff, dest);
	releaseAutoBlock(dest);

	CheckClogging(i, buff);
}

static void Perform(int sock, int fwdSock)
{
	cout("接続\n");

	CrossChannel(
		sock,
		fwdSock,
		SendFltrInfo ? StrmFltr : NULL,
		(uint)SendFltrInfo,
		RecvFltrInfo ? StrmFltr : NULL,
		(uint)RecvFltrInfo
		);

	cout("切断\n");
}
static int ReadArgs(void)
{
	if (argIs("/SF"))
	{
		SendFltrInfo = GetFltrInfo(nextArg(), "[SEND]");
		return 1;
	}
	if (argIs("/RF"))
	{
		RecvFltrInfo = GetFltrInfo(nextArg(), "[RECV]");
		return 1;
	}
	if (argIs("/T"))
	{
		CloggedTimeoutSec = toValue(nextArg());
		return 1;
	}
	return 0;
}
static char *GetTitleSuffix(void)
{
	uint sf = SendFltrInfo ? getCount(SendFltrInfo->SrcPtns) : 0;
	uint rf = RecvFltrInfo ? getCount(RecvFltrInfo->SrcPtns) : 0;

	return xcout("SF:%u RF:%u T:%u", sf, rf, CloggedTimeoutSec);
}
int main(int argc, char **argv)
{
	TunnelMain(ReadArgs, Perform, "Spoof", GetTitleSuffix);
}
