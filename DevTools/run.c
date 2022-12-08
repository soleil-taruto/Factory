/*
	run.exe [/D] [/-R] PROGRAM-NAME [PARAM]...

		/D ... プログラムの場所へ移動して実行する。

	- - -

	デフォルトでは実行するプログラムと同じディレクトリへ移動しないので注意！
*/

#include "C:\Factory\Common\all.h"

static int IntoProgramDirMode;
static int DebugMode;

static char *ToPrms(autoList_t *args)
{
	char *buff = strx("");
	char *arg;
	uint index;

	foreach (args, arg, index)
	{
		buff = addChar(buff, ' ');

		if (!*arg || strchr(arg, '\t') || strchr(arg, ' '))
		{
			buff = addChar(buff, '"');
			buff = addLine(buff, arg);
			buff = addChar(buff, '"');
		}
		else
		{
			buff = addLine(buff, arg);
		}
	}
	return buff;
}

static char *FP_Program;
static autoList_t *FP_Founds;

static void FindProgram_Collect(char *trgDir)
{
	char *batFile = xcout("%s\\%s.bat", trgDir, FP_Program);
	char *exeFile = xcout("%s\\%s.exe", trgDir, FP_Program);

	if (existFile(batFile))
	{
		addElement(FP_Founds, (uint)strx(batFile));
	}
	if (existFile(exeFile))
	{
		addElement(FP_Founds, (uint)strx(exeFile));
	}
	memFree(batFile);
	memFree(exeFile);
}
static void FindProgram_Main(char *trgDir)
{
	autoList_t *dirs = lsDirs(trgDir);
	char *dir;
	uint dir_index;

	FindProgram_Collect(trgDir);

	foreach (dirs, dir, dir_index)
	{
		char *localDir = getLocal(dir);

		if (lineExp("<1,,__09AZaz>", localDir) && _stricmp(localDir, "obj") && _stricmp(localDir, DebugMode ? "Release" : "Debug"))
		{
			char *subDir = xcout("%s\\%s", trgDir, localDir);

			FindProgram_Main(subDir);

			memFree(subDir);
		}
	}
	releaseDim(dirs, 1);
}
static char *FindProgram(char *program)
{
	FP_Program = program;
	FP_Founds = newList();

	FindProgram_Main(".");

	errorCase_m(getCount(FP_Founds) == 0, "プログラムが見つかりません。");

	if (getCount(FP_Founds) != 1)
	{
		char *found;
		uint found_index;

		foreach (FP_Founds, found, found_index)
		{
			cout("見つかったプログラム：%s\n", found);
		}
		error_m("複数のプログラムが見つかりました。");
	}

	program = getLine(FP_Founds, 0);

	FP_Program = NULL;
	releaseAutoList(FP_Founds);
	FP_Founds = NULL;

	return program;
}
int main(int argc, char **argv)
{
	char *program;
	char *prms;

	IntoProgramDirMode = argIs("/D");
	DebugMode = argIs("/-R");
	program = nextArg();

	errorCase(!lineExp("<1,,__09AZaz>", program));

	{
		autoList_t *args = allArgs();

		prms = ToPrms(args);

		releaseAutoList(args);
	}

	program = FindProgram(program);

	cout("%s%s\n", program, prms);

	if (IntoProgramDirMode)
	{
		char *wd = changeLocal(program, "");

		addCwd(wd);
		{
			execute_x(xcout("%s%s", getLocal(program), prms));
		}
		unaddCwd();

		memFree(wd);
	}
	else
	{
		execute_x(xcout("%s%s", program, prms));
	}

	memFree(program);
	memFree(prms);

	return lastSystemRet;
}
