/*
	wavCut4Game.exe [/F FADEOUT-TIME] [/FL FADEOUT-MILLIS] [/E END-SILENT-MILLIS] [/B BGN-SILENT-MILLIS] [WAV-FILE]

		/F  ... 省略時、フェードアウトしない。
		FADEOUT-TIME ... 秒 or 分:秒 or 分.秒
		/FL ... 省略時、900[ms]
		/E  ... 省略時、100[ms]
		/B  ... 省略時、100[ms]
*/

#include "C:\Factory\Common\all.h"
#include "libs\wav.h"

#define HZ 44100

static autoList_t *WavData;

static void Fadeout(uint startPos, uint count)
{
	uint index;

	for (index = 0; index < count; index++)
	{
		double rate = (double)(count - index) / count;
		uint value = getElement(WavData, startPos + index);
		uint v1;
		uint v2;

		v1 = value >> 16;
		v2 = value & 0xffff;

		v1 = m_d2i(((double)v1 - 0x8000) * rate + 0x8000);
		v2 = m_d2i(((double)v2 - 0x8000) * rate + 0x8000);

		value = v1 << 16 | v2;
		setElement(WavData, startPos + index, value);
	}
	setCount(WavData, startPos + index);
}
static void TrimEnd(void)
{
	while (getLastElement(WavData) == 0x80008000)
		unaddElement(WavData);
}
static void TrimBgn(void)
{
	uint rPos;
	uint wPos;

	for (rPos = 0; getElement(WavData, rPos) == 0x80008000; rPos++)
		noop();

	for (wPos = 0; rPos < getCount(WavData); wPos++, rPos++)
		setElement(WavData, wPos, getElement(WavData, rPos));

	setCount(WavData, wPos);
}
static void PutSilentEnd(uint count)
{
	while (count)
	{
		addElement(WavData, 0x80008000);
		count--;
	}
}
static void PutSilentBgn(uint count)
{
	autoList_t *nwd = newList();

	while (count)
	{
		addElement(nwd, 0x80008000);
		count--;
	}
	addElements(nwd, WavData);
	releaseAutoList(WavData);
	WavData = nwd;
}

static uint FadeoutPos; // 0 == 何もしない。
static uint FadeoutLen = (HZ * 9) / 10;
static uint SilentEndLen = HZ / 10;
static uint SilentBgnLen = HZ / 10;

static void WavCut4Game(char *rFile, char *wFile)
{
	WavData = readWAVFile(rFile);

	errorCase(lastWAV_Hz != HZ);

	if (FadeoutPos)
		Fadeout(FadeoutPos, FadeoutLen);

	TrimEnd();
	TrimBgn();
	PutSilentEnd(SilentEndLen);
	PutSilentBgn(SilentBgnLen);

	writeWAVFile(wFile, WavData, HZ);

	releaseAutoList(WavData);
	WavData = NULL;
}
static void WavCut4Game_F(char *file)
{
	char *rFile = file;
	char *wFile;

	wFile = strx(rFile);
	wFile = changeExt(wFile, "");
	wFile = addLine_x(wFile, xcout("_wc4g_[%u_%u_%u_%u].wav", FadeoutPos, FadeoutLen, SilentEndLen, SilentBgnLen));
	wFile = toCreatablePath(wFile, 1000);

	WavCut4Game(rFile, wFile);

	memFree(wFile);
}
static uint StrToSec(char *str)
{
	char *p;
	uint sec;

	str = strx(str);
	p = strchr(str, ':');

	if (!p)
		p = strchr(str, '.');

	if (p)
	{
		*p = '\0';
		p++;
		sec = toValue(str) * 60 + toValue(p);
	}
	else
	{
		sec = toValue(str);
	}
	memFree(str);
	return sec;
}
int main(int argc, char **argv)
{
	uint fadeoutPos;

readArgs:
	if (argIs("/F"))
	{
		FadeoutPos = StrToSec(nextArg()) * HZ;
		goto readArgs;
	}
	if (argIs("/FL"))
	{
		FadeoutLen = (toValue(nextArg()) * HZ) / 1000;
		goto readArgs;
	}
	if (argIs("/E"))
	{
		SilentEndLen = (toValue(nextArg()) * HZ) / 1000;
		goto readArgs;
	}
	if (argIs("/B"))
	{
		SilentBgnLen = (toValue(nextArg()) * HZ) / 1000;
		goto readArgs;
	}

	if (hasArgs(1))
	{
		char *file = nextArg();

		WavCut4Game_F(file);
		return;
	}

	{
		char *file = c_dropFile();

		WavCut4Game_F(file);
	}
}
