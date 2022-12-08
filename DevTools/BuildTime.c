/*
	BuildTime.exe [/C [DIR-1 DIR-2] | DIR]

	フォルダ内の実行ファイルのビルド日時を表示する。PE PEHeader PETime
	実行ファイル以外はMD5の前半64bitを表示する。
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\OpenSource\md5.h"

static void ShowBuildTime_File(char *file, char *file4Prn)
{
	autoBlock_t *fileData = readBinary(file);
	uint index;
	time_t buildTime;

	for (index = 0; ; index++)
	{
		if (
			getByte(fileData, index + 0) == 'P' &&
			getByte(fileData, index + 1) == 'E' &&
			getByte(fileData, index + 2) == '\0' &&
			getByte(fileData, index + 3) == '\0'
			)
			break;
	}
	index += 8; // ヘッダとか

	buildTime =
		getByte(fileData, index + 0) * 0x1 +
		getByte(fileData, index + 1) * 0x100 +
		getByte(fileData, index + 2) * 0x10000 +
		getByte(fileData, index + 3) * 0x1000000;

	cout("%s %s\n", makeJStamp(getStampDataTime(buildTime), 0), file4Prn);

	releaseAutoBlock(fileData);
}
static void ShowBuildTime(char *trgPath)
{
	trgPath = makeFullPath(trgPath);

	if (existDir(trgPath))
	{
		autoList_t *paths = lss(trgPath);
		char *path;
		uint index;

		sortJLinesICase(paths);

		foreach (paths, path, index)
		{
			char *relPath = changeRoot(strx(path), trgPath, NULL);

			if (existDir(path))
			{
				cout("------------------------ %s\n", relPath);
			}
			else if (
				!_stricmp("exe", getExt(path)) ||
				!_stricmp("dll", getExt(path)) ||
				!_stricmp("exe_", getExt(path)) ||
				!_stricmp("dll_", getExt(path))
				)
			{
				ShowBuildTime_File(path, relPath);
			}
			else
			{
				char *hash = md5_makeHexHashFile(path);

				hash[16] = '\0';

				cout("    %s     %s\n", hash, relPath);

				memFree(hash);
			}
			memFree(relPath);
		}
		releaseDim(paths, 1);
	}
	else
	{
		ShowBuildTime_File(trgPath, trgPath);
	}
	memFree(trgPath);
}
int main(int argc, char **argv)
{
	if (argIs("/C"))
	{
		char *outFile1 = getOutFile("BuildTime_Comp_1.txt");
		char *outFile2 = getOutFile("BuildTime_Comp_2.txt");
		char *path1;
		char *path2;

		if (hasArgs(2))
		{
			path1 = nextArg();
			path2 = nextArg();
		}
		else
		{
			path1 = dropPath(); // g
			path2 = dropPath(); // g
		}
		setCoutWrFile(outFile1, "wt");
		ShowBuildTime(path1);
		setCoutWrFile(outFile2, "wt");
		ShowBuildTime(path2);
		unsetCoutWrFile();

		if (isSameFile(outFile1, outFile2))
		{
			cout("+------+\n");
			cout("| 一致 |\n");
			cout("+------+\n");
		}
		else
		{
			cout("+--------+\n");
			cout("| 不一致 |\n");
			cout("+--------+\n");
		}
		openOutDir();
		return;
	}
	if (hasArgs(1))
	{
		ShowBuildTime(nextArg());
		return;
	}

	for (; ; )
	{
		ShowBuildTime(c_dropDirFile());
		cout("\n");
	}
}
