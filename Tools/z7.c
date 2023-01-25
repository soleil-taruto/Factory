/*
	.zip .7z の圧縮のみ

	- - -

	z7.exe [/C] [/D 出力ファイル] [/T] [/7] [/OAD] [/P パスフレーズ] [入力ファイル | 入力DIR]

		/C   ... 入力ファイルと同じ場所に圧縮する。
		/D   ... 出力ファイル名を指定する。
		/T   ... 不要な上位階層を除去する。(DIRのときのみ)
		/7   ... .7z にする。
		/OAD ... 元ファイル・ディレクトリ自動削除
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Collabo.h"
#include "C:\Factory\Meteor\7z.h"

static int OutputSameDir;
static char *OutputFile;
static int TrimTopDir;
static int WFileType = 'Z'; // "7Z"
static int OutputAndDelete;
static char *Passphrase;

static char *Get7zExeFile(void) // ret: 空白を含まないパスであること。
{
	static char *file;

	if (!file)
		file = GetCollaboFile(FILE_7Z_EXE);

	return file;
}
static char *GetSwitches(void)
{
	static char *ret;

	memFree(ret);
	ret = strx("");

	if (Passphrase)
	{
		errorCase_m(!lineExp("<1,100,--__09AZaz>", Passphrase), "パスフレーズに使える文字は - _ 0～9 A～Z a～z です。");

		ret = addLine_x(ret, xcout(" -p%s", Passphrase));
	}
	return ret;
}

static char *LastOutputDir;

static char *GetWFile(char *path)
{
	char *file7z;
	char *ext;

	cout("< %s\n", path);

	switch (WFileType)
	{
	case '7': ext = "7z";  break;
	case 'Z': ext = "zip"; break;

	default:
		error();
	}

	if (OutputSameDir)
	{
		file7z = addExt(strx(path), ext);
		file7z = toCreatableTildaPath(file7z, IMAX);
	}
	else if (OutputFile)
	{
		errorCase_m(_stricmp(ext, getExt(OutputFile)), "出力形式と拡張子が一致していません。拡張子 = zip OR 7z");

		file7z = strx(OutputFile);
	}
	else
	{
		char *dir = makeFreeDir();

		file7z = combine(dir, getLocal(path));
		file7z = addExt(file7z, ext);

		memFree(LastOutputDir);
		LastOutputDir = dir;
	}

	cout("> %s\n", file7z);

	return file7z;
}
static void Pack7z_File(char *file)
{
	char *file7z = GetWFile(file);

	coExecute_x(xcout("%s a%s \"%s\" \"%s\"", Get7zExeFile(), GetSwitches(), file7z, file));

	memFree(file7z);

	LOGPOS();
}
static void Pack7z_Dir(char *dir)
{
	char *file7z;

	errorCase(isAbsRootDir(dir)); // ルートDIR 不可

	file7z = GetWFile(dir);

	if (TrimTopDir)
	{
		autoList_t *subPaths = ls(dir);
		char *subPath;
		uint index;

		foreach (subPaths, subPath, index)
		{
			coExecute_x(xcout("%s a%s \"%s\" \"%s\"", Get7zExeFile(), GetSwitches(), file7z, subPath));
		}
		releaseDim(subPaths, 1);
	}
	else
	{
		coExecute_x(xcout("%s a%s \"%s\" \"%s\"", Get7zExeFile(), GetSwitches(), file7z, dir));
	}
	memFree(file7z);

	LOGPOS();
}
static void Pack7z(char *path)
{
	path = makeFullPath(path);

	if (existFile(path))
	{
		Pack7z_File(path);
	}
	else if (existDir(path))
	{
		Pack7z_Dir(path);
	}
	else
	{
		error();
	}
	if (LastOutputDir)
	{
		LOGPOS();
		coExecute_x(xcout("START %s", LastOutputDir));
	}
	if (OutputAndDelete)
	{
		LOGPOS();
		recurRemovePath(path);
	}
	memFree(path);

	LOGPOS();
}
int main(int argc, char **argv)
{
	cout("=======\n");
	cout("z7 圧縮\n");
	cout("=======\n");

readArgs:
	if (argIs("/C"))
	{
		cout("+----------------+\n");
		cout("| 同じ場所に作成 |\n");
		cout("+----------------+\n");

		OutputSameDir = 1;
		goto readArgs;
	}
	if (argIs("/D"))
	{
		// memo: Release.bat のために追加した。
		// 出力ファイル名を指定するような使い方は SubTools/zip.c 向き。

		OutputFile = makeFullPath(nextArg());

		cout("+--------------------------------+\n");
		cout("| 出力ファイル名が指定されました |\n");
		cout("+--------------------------------+\n");
		cout("* %s\n", OutputFile);

		// 試し書き + 既存ファイルを削除
		{
			createFile(OutputFile);
			removeFile(OutputFile);
		}

		goto readArgs;
	}
	if (argIs("/T"))
	{
		TrimTopDir = 1;
		goto readArgs;
	}
	if (argIs("/7"))
	{
		WFileType = '7';
		goto readArgs;
	}
	if (argIs("/OAD"))
	{
		cout("+-------------------+\n");
		cout("| OUTPUT AND DELETE |\n");
		cout("+-------------------+\n");

		OutputAndDelete = 1;
		goto readArgs;
	}
	if (argIs("/P"))
	{
		Passphrase = nextArg();
		goto readArgs;
	}

	if (hasArgs(1))
	{
		Pack7z(nextArg());
		return;
	}

	for (; ; )
	{
		Pack7z(c_dropDirFile());

		cout("\n");
		cout("-------\n");
		cout("z7 圧縮\n");
		cout("-------\n");
	}
}
