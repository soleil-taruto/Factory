/*
	Stamp.exe [/-S] DIR [FILE]

		スタンプリストを作成

		/-S  ... 入力DIRの直下のみ
		DIR  ... 入力DIR
		FILE ... 出力ファイル, 省略時は DIR + ".stamp"

	Stamp.exe FILE [DIR]

		スタンプリストを反映

		FILE ... 入力ファイル
		DIR  ... 適用先DIR, 省略時は FILE - .ext
*/

#include "C:\Factory\Common\all.h"

#define STAMPLIST_EXT "stamp"

/*
	dir: absDir && !rootDir
	stampListFile: NULL || absFile
*/
static void MakeStampList(char *dir, char *stampListFile, int subDirEnabled)
{
	autoList_t *files = (subDirEnabled ? lssFiles : lsFiles)(dir);
	char *file;
	uint index;
	autoList_t *lines = newList();

	if (!stampListFile)
		stampListFile = addExt(strx(dir), STAMPLIST_EXT);

	if (existFile(stampListFile))
	{
		cout("> %s\n", stampListFile);
		cout("OVER-WRITE OK?\n");

		if (clearGetKey() == 0x1b)
			termination(0);

		cout("OK\n");
	}

	changeRoots(files, dir, NULL);
	sortJLinesICase(files);

	addCwd(dir);

	foreach (files, file, index)
	{
		uint64 createTime;
		uint64 accessTime;
		uint64 updateTime;

		getFileStamp(file, &createTime, &accessTime, &updateTime);

		cout("< %s\n", file);
		cout("CREATE TIME %I64u\n", createTime);
		cout("ACCESS TIME %I64u\n", accessTime);
		cout("UPDATE TIME %I64u\n", updateTime);

		addElement(lines, (uint)xcout("%I64u %I64u %I64u %s", createTime, accessTime, updateTime, file));
	}
	unaddCwd();

	writeLines(stampListFile, lines);

	memFree(stampListFile);
	releaseDim(files, 1);
	releaseDim(lines, 1);
}
/*
	stampListFile: absFile
	dir: NULL || absDir
*/
static void ApplyStampList(char *stampListFile, char *dir)
{
	autoList_t *lines = readLines(stampListFile);
	char *line;
	uint index;

	if (!dir)
		dir = changeExt(stampListFile, "");

	addCwd(dir);

	foreach (lines, line, index)
	{
		autoList_t *tokens = tokenizeMinMax(line, '\x20', 4, 4, "*");
		uint64 createTime;
		uint64 accessTime;
		uint64 updateTime;
		char *file;

		createTime = toValue64(getLine(tokens, 0));
		accessTime = toValue64(getLine(tokens, 1));
		updateTime = toValue64(getLine(tokens, 2));
		file = getLine(tokens, 3);

		cout("CREATE TIME %I64u\n", createTime);
		cout("ACCESS TIME %I64u\n", accessTime);
		cout("UPDATE TIME %I64u\n", updateTime);
		cout("> %s\n", file);

		if (existFile(file))
		{
			/*
				不正な日時は setFileStamp() 内で error() にしてくれるはず。
			*/
			setFileStamp(file, createTime, accessTime, updateTime);
		}
		else
		{
			cout("ファイルが存在しないのでスキップします。\n");
		}
		releaseDim(tokens, 1);
	}
	unaddCwd();

	memFree(dir);
	releaseDim(lines, 1);
}
int main(int argc, char **argv)
{
	int subDirEnabled = 1;
	char *path;
	char *subPath;

	if (argIs("/-S"))
	{
		subDirEnabled = 0;
	}

	if (hasArgs(1))
	{
		path = nextArg();
	}
	else
	{
		path = c_dropPath();

		if (!path)
			termination(0);
	}
	path = makeFullPath(path);

	if (hasArgs(1))
	{
		subPath = nextArg();
		subPath = makeFullPath(subPath);
	}
	else
	{
		subPath = NULL;
	}

	if (existDir(path))
	{
		errorCase(isAbsRootDir(path));
		errorCase(subPath && !existFile(subPath));

		MakeStampList(path, subPath, subDirEnabled);
	}
	else
	{
		errorCase(!existFile(path));
		errorCase(subPath && !existDir(subPath));

		ApplyStampList(path, subPath);
	}
	memFree(path);
}
