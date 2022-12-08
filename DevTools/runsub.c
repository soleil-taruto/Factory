/*
	> runsub.exe [/-B] [/-S | /-R] [/-O] [/R ROOT-DIR] [/T TIMEWAIT-SEC] [/TM TIMEWAIT-MILLIS] TARGET

		/-B             ... ���s����v���O�����̏I����҂����Ɏ��֐i�ށB
		/-S             ... �T�u�f�B���N�g���𖳎�����B
		/-R             ... �����̃��[�g�f�B���N�g���𖳎�����B
		/-O             ... Ignore obj Debug
		ROOT-DIR        ... �����̃��[�g�f�B���N�g�����w�肷��B�f�t�H = ���s���̃J�����g
		TIMEWAIT-SEC    ... �v���O���������s����x�ɑ҂b���B
		TIMEWAIT-MILLIS ... �v���O���������s����x�ɑ҂~���b���B
		TARGET          ... ���s����v���O�����̃m�[�h��
		                    �S�Ẵv���O���������s����ꍇ�� * ���w�肷��B
		                    *PTN �Ƃ���ƕ�����v�ɂȂ�B

	- - -

	���s����v���O�����Ɠ����f�B���N�g���ֈړ����Ă�����s����B
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
		if (!existDir(dir)) // ����܂ł� Run() �ɂ���č폜����邱�Ƃ�����B
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
