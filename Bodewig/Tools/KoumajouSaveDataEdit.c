/*
	KoumajouSaveDataEdit.exe [/D ゲームDIR] [/S ステージ番号] [/H ハイスコア] [/E+ | /E-]

		ステージ番号 ... 1 ～ 8         --  範囲外は最初からになるっぽい。
		ハイスコア   ... 0 ～ 99999999  --  範囲外は上位桁が捨てられるっぽい。
		/E+          ... エクストラ開放
		/E-          ... エクストラ未開放

	- - -
	例

		KoumajouSaveDataEdit.exe /D C:\game\koumajou /S 8 /E+

		KoumajouSaveDataEdit.exe /H 13413983

	- - -

	ver.1.04a で検証した。
*/

#include "C:\Factory\Common\all.h"

#define S_MASK "055b2d160639034d010b"
#define SIGNATURE "This file is save data of Scarlet Symphony."
#define EXTRA_OPENED "extramode_enable"
#define NEW_LINE "\r\n"

static char *GameDir = "C:\\home\\Game\\紅魔城伝説";

static uint StageNo = 1;
static uint HiScore = 0;
static int ExtraOpened = 0;

static char *GetSaveDataFile(void)
{
	return combine(GameDir, "data\\savedata.dat");
}
static void DoMask(autoBlock_t *fileData)
{
	autoBlock_t *mask = makeBlockHexLine(S_MASK);
	uint index;

	for (index = 0; index < getSize(fileData); index++)
	{
		b_(fileData)[index] ^= b_(mask)[index % getSize(mask)];
	}
	releaseAutoBlock(mask);
}
static void ShowSaveData(void)
{
	cout("StageNo: %u\n", StageNo);
	cout("HiScore: %u\n", HiScore);
	cout("ExtraOpened: %d\n", m_01(ExtraOpened));
}
static void LoadSaveData(void)
{
	char *file = GetSaveDataFile();

	if (existFile(file))
	{
		autoBlock_t *fileData = readBinary(file);
		char *fileText;
		autoList_t *lines;
		autoList_t *sVals;

		DoMask(fileData);
		fileText = unbindBlock2Line(fileData);
		fileText = replaceLine(fileText, "\r\n", "\n", 0);
		lines = tokenize(fileText, '\n');
		sVals = tokenize(getLine(lines, 1), ',');
		StageNo = toValue(refLine(sVals, 0));
		HiScore = toValue(refLine(sVals, 1));
		ExtraOpened = getCount(lines) == 3;
		memFree(fileText);
		releaseDim(lines, 1);
		releaseDim(sVals, 1);
	}
	memFree(file);
}
static void OutputSaveData(void)
{
	char *fileText = strx(SIGNATURE NEW_LINE);
	autoBlock_t *fileData;

	fileText = addLine_x(fileText, xcout("%u,%u", StageNo, HiScore));

	if (ExtraOpened)
		fileText = addLine(fileText, NEW_LINE EXTRA_OPENED);

	fileData = ab_makeBlockLine_x(fileText);
	DoMask(fileData);
	writeBinary_xx(GetSaveDataFile(), fileData);
}
int main(int argc, char **argv)
{
	int changed = 0;

	if (argIs("/D"))
	{
		GameDir = nextArg();
		goto readArgs;
	}

	// Check 'GameDir'
	{
		char *file;

		errorCase_m(!existDir(GameDir), "Wrong Game-Dir");

		file = combine(GameDir, "koumajou.exe");
		errorCase_m(!existFile(file), "Wrong Game-Dir, koumajou.exe does not exist!");
		memFree(file);

		file = combine(GameDir, "data");
		errorCase_m(!existDir(file), "Wrong Game-Dir, data does not exist!");
		memFree(file);
	}

	LoadSaveData();
	ShowSaveData();

readArgs:
	if (argIs("/S"))
	{
		StageNo = toValue(nextArg());
		changed = 1;
		goto readArgs;
	}
	if (argIs("/H"))
	{
		HiScore = toValue(nextArg());
		changed = 1;
		goto readArgs;
	}
	if (argIs("/E+"))
	{
		ExtraOpened = 1;
		changed = 1;
		goto readArgs;
	}
	if (argIs("/E-"))
	{
		ExtraOpened = 0;
		changed = 1;
		goto readArgs;
	}

	if (changed)
	{
		cout(">\n");

		ShowSaveData();
		OutputSaveData();
	}
}
