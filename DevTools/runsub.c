/*
	> runsub.exe [/-B] [/-S | /-R] [/-O] [/R ROOT-DIR] [/T TIMEWAIT-SEC] [/TM TIMEWAIT-MILLIS] TARGET

		/-B             ... 実行するプログラムの終了を待たずに次へ進む。
		/-S             ... サブディレクトリを無視する。
		/-R             ... 検索のルートディレクトリを無視する。
		/-O             ... Ignore obj Debug
		ROOT-DIR        ... 検索のルートディレクトリを指定する。デフォ = 実行時のカレント
		TIMEWAIT-SEC    ... プログラムを実行する度に待つ秒数。
		TIMEWAIT-MILLIS ... プログラムを実行する度に待つミリ秒数。
		TARGET          ... 実行するプログラムのノード名
		                    全てのプログラムを実行する場合は * を指定する。
		                    *PTN とすると部分一致になる。

	- - -

	実行するプログラムと同じディレクトリへ移動してから実行する。
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\SubTools\BlueFish\libs\Lock.h"

static uint NonBlockingMode;
static uint TimeWaitMillis;

static void Run(char *file)
{
	char *absPath = c_makeFullPath(file);

	cout("runsub: %s\n", absPath);
	cmdTitle_x(xcout("runsub - %s", absPath));

	BlueFish_Lock();
	{
		if (NonBlockingMode)
			execute_x(xcout("START CMD /C \"%s\"", file));
		else
			execute(file);
	}
	BlueFish_Unlock();

	cmdTitle("runsub");
	cout("runsub: %s done\n", absPath);

	if (TimeWaitMillis)
		coSleep(TimeWaitMillis);
}
int main(int argc, char **argv)
{
	int intoSubDirMode = 1;
	int skipRootDir = 0;
	int ignoreObjDebug = 0;
	char *rootDir = ".";
	char *target;
	char *batch;
	char *execu;
	autoList_t *dirs;
	char *dir;
	uint index;

readArgs:
	if (argIs("/-B"))
	{
		NonBlockingMode = 1;
		goto readArgs;
	}
	if (argIs("/-S"))
	{
		intoSubDirMode = 0;
		goto readArgs;
	}
	if (argIs("/-R"))
	{
		skipRootDir = 1;
		goto readArgs;
	}
	if (argIs("/-O"))
	{
		ignoreObjDebug = 1;
		goto readArgs;
	}
	if (argIs("/R"))
	{
		rootDir = nextArg();
		goto readArgs;
	}
	if (argIs("/T"))
	{
		TimeWaitMillis = toValue(nextArg()) * 1000;
		goto readArgs;
	}
	if (argIs("/TM"))
	{
		TimeWaitMillis = toValue(nextArg());
		goto readArgs;
	}
	target = nextArg();

	batch = addExt(strx(target), "bat");
	execu = addExt(strx(target), "exe");

	if (intoSubDirMode)
	{
		dirs = lssDirs(rootDir);
		sortJLinesICase(dirs);

		if (!skipRootDir)
			insertElement(dirs, 0, (uint)strx(rootDir));
	}
	else
	{
		dirs = createOneElement((uint)strx(rootDir));
	}

	foreach (dirs, dir, index)
	{
		if (!existDir(dir)) // これまでの Run() によって削除されることもある。
			continue;

		if (ignoreObjDebug && (mbs_stristr(dir, "\\obj\\") || mbs_stristr(dir, "\\Debug\\")))
			continue;

		addCwd(dir);

		if (target[0] == '*')
		{
			autoList_t *files = lsFiles(".");
			char *file;
			uint file_index;

			eraseParents(files);
			sortJLinesICase(files);

			foreach (files, file, file_index)
			{
				if (
					!_stricmp("bat", getExt(file)) ||
					!_stricmp("exe", getExt(file))
					)
				{
					if (target[1] == '\0' || mbs_strstrCase(file, target + 1, 1))
					{
						Run(file);
					}
				}
			}
			releaseDim(files, 1);
		}
		else
		{
			if (existFile(batch))
			{
				Run(batch);
			}
			if (existFile(execu))
			{
				Run(execu);
			}
		}
		unaddCwd();
	}
	releaseDim(dirs, 1);
	memFree(batch);
	memFree(execu);
}
