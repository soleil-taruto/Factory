/*
	FindBmpOnScreen.exe [/W] [/M 色マージン] BMPファイル

	エラーレベル
		0 ... 見つかった。
		1 ... 見つからなかった。
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\SubTools\libs\bmptbl.h"
#include "libs\PrintScreen.h"

static uint ColorLvMargin = 10;

static autoTable_t *TrgBmp;
static uint TrgBmp_W;
static uint TrgBmp_H;

// FCD == First Check Dot

#define FCDNUMMAX 10

static uint FCDNum;
static uint FCDXs[FCDNUMMAX];
static uint FCDYs[FCDNUMMAX];

static int IsSameDot(uint a, uint b)
{
	sint aR = a >> 16;
	sint aG = a >> 8 | 0xff;
	sint aB = a | 0xff;
	sint bR = b >> 16;
	sint bG = b >> 8 | 0xff;
	sint bB = b | 0xff;

	return
		abs(aR - bR) <= ColorLvMargin ||
		abs(aG - bG) <= ColorLvMargin ||
		abs(aB - bB) <= ColorLvMargin;
}
static int IsMatchBmpArea(autoTable_t *bmp, uint l, uint t)
{
	uint i;
	uint x;
	uint y;

	for (i = 0; i < FCDNum; i++)
		if (!IsSameDot(getTableCell(bmp, l + FCDXs[i], t + FCDYs[i]), getTableCell(TrgBmp, FCDXs[i], FCDYs[i])))
			return 0;

	for (x = 0; x < TrgBmp_W; x++)
	for (y = 0; y < TrgBmp_H; y++)
	{
		if (!IsSameDot(getTableCell(bmp, l + x, t + y), getTableCell(TrgBmp, x, y)))
			return 0;
	}
	return 1;
}
static int FindBmp_Bmp(autoTable_t *bmp)
{
	uint w = getTableWidth(bmp);
	uint h = getTableHeight(bmp);
	uint x;
	uint y;

	for (x = 0; x + TrgBmp_W <= w; x++)
	for (y = 0; y + TrgBmp_H <= h; y++)
	{
		if (IsMatchBmpArea(bmp, x, y))
			return 1;
	}
	return 0;
}
static int FindBmp_Screen(autoBlock_t *bmp)
{
	char *file = makeTempPath("bmp");
	autoTable_t *tBmp;
	int ret;

	writeBinary(file, bmp);
	tBmp = tReadBMPFile(file);
	removeFile(file);

	ret = FindBmp_Bmp(tBmp);

	memFree(file);
	releaseTable(tBmp);
	cout("ret: %d\n", ret);
	return ret;
}
static int FindBmpOnScreen(void)
{
	autoList_t *bmps = PrintScreen();
	autoBlock_t *bmp;
	uint index;
	int ret = 0;

	foreach (bmps, bmp, index)
	{
		if (FindBmp_Screen(bmp))
		{
			ret = 1;
			break;
		}
	}
	releaseDim_BR(bmps, 1, (void (*)(void *))releaseAutoBlock);
	return ret;
}
static void InitFCD(void)
{
	uint x;
	uint y;

	FCDXs[0] = 0;
	FCDYs[0] = 0;
	FCDNum = 1;

	for (x = 0; x < TrgBmp_W; x++)
	for (y = 0; y < TrgBmp_H; y++)
	{
		uint i;

		for (i = 0; i < FCDNum; i++)
			if (IsSameDot(getTableCell(TrgBmp, FCDXs[i], FCDYs[i]), getTableCell(TrgBmp, x, y)))
				goto nextLoop;

		cout("FCDX_Ys: %u, %u\n", x, y);

		FCDXs[FCDNum] = x;
		FCDYs[FCDNum] = y;
		FCDNum++;

		if (FCDNUMMAX <= FCDNum)
			goto endLoop;

	nextLoop:;
	}
endLoop:;
}
int main(int argc, char **argv)
{
	int waitLoopMode = 0;

readArgs:
	if (argIs("/W"))
	{
		waitLoopMode = 1;
		goto readArgs;
	}
	if (argIs("/M"))
	{
		ColorLvMargin = toValue(nextArg());
		goto readArgs;
	}

	TrgBmp = tReadBMPFile(nextArg());
	TrgBmp_W = getTableWidth(TrgBmp);
	TrgBmp_H = getTableHeight(TrgBmp);

	errorCase_m(hasArgs(1), "不明なコマンド引数");

	InitFCD();

	if (waitLoopMode)
	{
		uint sec = 0;

		LOGPOS();

		while (!FindBmpOnScreen())
		{
			if (sec < 20)
				sec++;

			LOGPOS();

			if (waitKey(sec * 1000) == 0x1b)
			{
				cout("ESCAPE key pressed.\n");
				termination(1);
			}
			LOGPOS();
		}
		LOGPOS();
		termination(0);
	}
	else
	{
		termination(FindBmpOnScreen() ? 0 : 1);
	}
}
