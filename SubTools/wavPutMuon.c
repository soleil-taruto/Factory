/*
	wavPutMuon.exe FIRST-MUON-MILLIS END-MUON-MILLIS [WAV-FILE]

		FIRST-MUON-MILLIS ... æ“ª‚Ì–³‰¹—Ìˆæ‚Ì’·‚³(ƒ~ƒŠ•b)
		END-MUON-MILLIS   ... I’[‚Ì–³‰¹—Ìˆæ‚Ì’·‚³(ƒ~ƒŠ•b)
*/

#include "C:\Factory\Common\all.h"
#include "libs\wav.h"

#define HZ 44100
#define MUON_LEVEL 32768
#define MUON_LEVEL_MARGIN 100
#define FADE_SZ (HZ / 10)

static autoList_t *WavData;
static uint FrontMuonMillis;
static uint EndMuonMillis;

static int IsMuonRow(uint row)
{
	uint l = row >> 16;
	uint r = row & 0xffff;

	return
		m_isRange(l,
			MUON_LEVEL - MUON_LEVEL_MARGIN,
			MUON_LEVEL + MUON_LEVEL_MARGIN
			) &&
		m_isRange(r,
			MUON_LEVEL - MUON_LEVEL_MARGIN,
			MUON_LEVEL + MUON_LEVEL_MARGIN
			);
}
static void RemoveEndMuon(void)
{
	uint count = getCount(WavData);

	while (count && IsMuonRow(getElement(WavData, count - 1)))
		count--;

	count += FADE_SZ;
	m_minim(count, getCount(WavData));
	setCount(WavData, count);
}
static void RemoveFrontMuon(void)
{
	reverseElements(WavData);
	RemoveEndMuon();
	reverseElements(WavData); // restore
}
static void AddEndMuon_Millis(uint millis)
{
	uint sampleCount = (uint)(((uint64)millis * HZ) / 1000);
	uint index;

	for (index = 0; index < sampleCount; index++)
		addElement(WavData, (MUON_LEVEL << 16) | MUON_LEVEL);
}
static void AddFrontMuon(void)
{
	reverseElements(WavData);
	AddEndMuon_Millis(FrontMuonMillis);
	reverseElements(WavData); // restore
}
static void AddEndMuon(void)
{
	AddEndMuon_Millis(EndMuonMillis);
}
static void SetEndFadeOut(void)
{
	uint index;
	uint pos;
	uint row;
	sint l;
	sint r;
	double rate;

	errorCase(getCount(WavData) < FADE_SZ);

	for (index = 1; index < FADE_SZ; index++)
	{
		pos = getCount(WavData) - index;
		row = getElement(WavData, pos);
		l = (sint)(row >> 16);
		r = (sint)(row & 0xffff);
		rate = (double)index / FADE_SZ;

		l -= MUON_LEVEL;
		l = d2i(l * rate);
		l += MUON_LEVEL;

		r -= MUON_LEVEL;
		r = d2i(r * rate);
		r += MUON_LEVEL;

		row = ((uint)l << 16) | (uint)r;

		setElement(WavData, pos, row);
	}
}
static void SetFrontFadeIn(void)
{
	reverseElements(WavData);
	SetEndFadeOut();
	reverseElements(WavData); // restore
}
static void PutMuon_D(void)
{
	RemoveFrontMuon();
	RemoveEndMuon();
	SetFrontFadeIn();
	SetEndFadeOut();
	AddFrontMuon();
	AddEndMuon();
}
static void PutMuon(char *rFile, char *wFile)
{
	WavData = readWAVFile(rFile);

	errorCase(lastWAV_Hz != HZ);

	PutMuon_D();

//	writeWAVFile(wFile, WavData, lastWAV_Hz);
	writeWAVFile(wFile, WavData, HZ);

	releaseAutoList(WavData);
	WavData = NULL;
}
static void PutMuon_F(char *file)
{
	char *rFile = file;
	char *wFile;

	wFile = strx(rFile);
	wFile = changeExt(wFile, "");
	wFile = addLine_x(wFile, xcout("_muon-%u-%u.wav", FrontMuonMillis, EndMuonMillis));
	wFile = toCreatableTildaPath(wFile, 1000);

	PutMuon(rFile, wFile);

	memFree(wFile);
}
int main(int argc, char **argv)
{
	FrontMuonMillis = toValue(nextArg());
	EndMuonMillis   = toValue(nextArg());

	errorCase(!m_isRange(FrontMuonMillis, 0, IMAX));
	errorCase(!m_isRange(EndMuonMillis,   0, IMAX));

	if (hasArgs(1))
	{
		char *file = nextArg();

		PutMuon_F(file);
		return;
	}

	for (; ; )
	{
		char *file = c_dropFile();

		PutMuon_F(file);
		cout("\n");
	}
}
