/*
	wavPutMuon.exe FIRST-MUON-MILLIS END-MUON-MILLIS [WAV-FILE]

		FIRST-MUON-MILLIS ... 先頭の無音領域の長さ(ミリ秒)
		END-MUON-MILLIS   ... 終端の無音領域の長さ(ミリ秒)
*/

#include "C:\Factory\Common\all.h"
#include "libs\wav.h"

#define HZ 44100
#define MUON_LEVEL 32768
#define MUON_ROW ((MUON_LEVEL << 16) | MUON_LEVEL)

static autoList_t *WavData;
static uint FrontMuonMillis;
static uint EndMuonMillis;

static void RemoveEndMuon(void)
{
	while(getCount(WavData) && getLastElement(WavData) == MUON_ROW)
		unaddElement(WavData);
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

	for(index = 0; index < sampleCount; index++)
		addElement(WavData, MUON_ROW);
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
static void PutMuon_D(void)
{
	RemoveFrontMuon();
	RemoveEndMuon();
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
	wFile = toCreatablePath(wFile, 1000);

	PutMuon(rFile, wFile);

	memFree(wFile);
}
int main(int argc, char **argv)
{
	FrontMuonMillis = toValue(nextArg());
	EndMuonMillis   = toValue(nextArg());

	errorCase(!m_isRange(FrontMuonMillis, 0, IMAX));
	errorCase(!m_isRange(EndMuonMillis,   0, IMAX));

	if(hasArgs(1))
	{
		char *file = nextArg();

		PutMuon_F(file);
		return;
	}

	for(; ; )
	{
		char *file = c_dropFile();

		PutMuon_F(file);
		cout("\n");
	}
}
