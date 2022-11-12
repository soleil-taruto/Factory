/*
	KoumajouStageSelect.exe [/D ゲームDIR] ステージ番号

	- - -

	ver.1.04a で検証した。
*/

#include "C:\Factory\Common\all.h"

static char *GameDir = "C:\\home\\Game\\紅魔城伝説";

static char *GetSaveDataFile(void)
{
	return combine(GameDir, "data\\savedata.dat");
}
static void ChangeStage(uint stage)
{
	char *file = GetSaveDataFile();
	autoBlock_t *fileData;

	errorCase_m(!m_isRange(stage, 1, 8), "Wrong Stage-Number");

	fileData = readBinary(file);

	setByte(fileData, 0x2d, stage ^ 0x09);

	writeBinary(file, fileData);
	memFree(file);
}
int main(int argc, char **argv)
{
readArgs:
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

		file = GetSaveDataFile();
		errorCase_m(!existFile(file), "Wrong Game-Dir, savedata.dat does not exist!");
		memFree(file);
	}

	{
		ChangeStage(toValue(nextArg()));
		return;
	}
}
