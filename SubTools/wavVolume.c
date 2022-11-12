/*
	wavVolume.exe VOLUME-PCT [WAV-FILE]

		VOLUME-PCT ... 元の音量の何パーセントか。0 ～ 65000
*/

#include "C:\Factory\Common\all.h"
#include "libs\wav.h"

#define HZ 44100

static autoList_t *WavData;
static uint VolumePct = 100;

static uint ChangeVol(uint index, uint uVol)
{
	sint vol;

	errorCase(0xffff < uVol);

	vol = (sint)uVol - 0x8000;

#if 0 // test
	vol = d2i((vol * (sint)VolumePct) / 100.0);
#else
	vol *= VolumePct;
	vol = divRndOff(vol, 100);
#endif

	if (!m_isRange(vol, -0x8000, 0x7fff))
	{
		cout("音割れPOI: [%u] %d\n", index, vol);
		m_range(vol, -0x8000, 0x7fff);
	}
	uVol = (uint)(vol + 0x8000);

	return uVol;
}
static void WavVolume_D(void)
{
	uint index;

	cout("VolumePct: %u\n", VolumePct);

	for (index = 0; index < getCount(WavData); index++)
	{
		uint value = getElement(WavData, index);
		uint v1;
		uint v2;

		v1 = value >> 16;
		v2 = value & 0xffff;

		v1 = ChangeVol(index, v1);
		v2 = ChangeVol(index, v2);

		value = v1 << 16 | v2;
		setElement(WavData, index, value);
	}
	setCount(WavData, index);
}
static void WavVolume(char *rFile, char *wFile)
{
	WavData = readWAVFile(rFile);

//	errorCase(lastWAV_Hz != HZ);

	WavVolume_D();

	writeWAVFile(wFile, WavData, lastWAV_Hz);
//	writeWAVFile(wFile, WavData, HZ);

	releaseAutoList(WavData);
	WavData = NULL;
}
static void WavVolume_F(char *file)
{
	char *rFile = file;
	char *wFile;

	wFile = strx(rFile);
	wFile = changeExt(wFile, "");
	wFile = addLine_x(wFile, xcout("_wv_[%u].wav", VolumePct));
	wFile = toCreatablePath(wFile, 1000);

	WavVolume(rFile, wFile);

	memFree(wFile);
}
int main(int argc, char **argv)
{
	VolumePct = toValue(nextArg());

	errorCase(!m_isRange(VolumePct, 0, 65000));

	if (hasArgs(1))
	{
		char *file = nextArg();

		WavVolume_F(file);
		return;
	}

	for (; ; )
	{
		char *file = c_dropFile();

		WavVolume_F(file);
		cout("\n");
	}
}
