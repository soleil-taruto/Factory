/*
	CacheSlim.exe [/D TIMEOUT-DAY | /H TIMEOUT-HOUR] CACHE-DIR "DELETE-OLD-CACHE"

		TIMEOUT-DAY  ... ���̓����ȏ�̂̃t�@�C���͍폜����B
		TIMEOUT-HOUR ... ���̎��Ԉȏ�̂̃t�@�C���͍폜����B
*/

#include "C:\Factory\Common\all.h"

static time_t TimeoutSec = 86400 * 7ui64; // 1 week
static char *RootDir;

typedef struct FileInfo_st
{
	char *File;
	time_t CWTime;
}
FileInfo_t;

static autoList_t *FileInfos;

static FileInfo_t *CreateFileInfo(char *file)
{
	FileInfo_t *i = nb_(FileInfo_t);
	uint64 c;
	uint64 w;
	uint64 cw;

	i->File = strx(file);
	getFileStamp(file, &c, NULL, &w);
	cw = m_max(c, w);
	i->CWTime = getTimeByFileStamp(cw);

	return i;
}
static void ReleaseFileInfo(FileInfo_t *i)
{
	memFree(i->File);
	memFree(i);
}
static void InitFileInfos(void)
{
	autoList_t *files;
	char *file;
	uint index;

	LOGPOS();

	files = cmdDir_lssFiles(RootDir);
	FileInfos = newList();

	foreach (files, file, index)
		addElement(FileInfos, (uint)CreateFileInfo(file));

	releaseDim(files, 1);

	LOGPOS();
}

// ---- slim ----

static time_t CurrTime;

static void SlimFile(FileInfo_t *i)
{
	time_t timeOld = CurrTime - i->CWTime;

	cout("%s\n", i->File);

	if (timeOld < 3600)
		cout("%I64d �b", timeOld);
	else if (timeOld < 86400)
		cout("%.3f ����", timeOld / 3600.0);
	else
		cout("%.3f ��", timeOld / 86400.0);

	cout("�O�ɐ����E�X�V���ꂽ�t�@�C���ł��B\n");

	if (TimeoutSec <= timeOld)
	{
		cout("�폜���܂��B\n");
		removeFile(i->File);
	}
	else
	{
		cout("�L�[�v���܂��B\n");
	}
}
static void SlimFiles(void)
{
	FileInfo_t *i;
	uint index;

	CurrTime = time(NULL);

	foreach (FileInfos, i, index)
	{
		SlimFile(i);
	}
}

// ----

int main(int argc, char **argv)
{
readArgs:
	if (argIs("/D"))
	{
		TimeoutSec = toValue64(nextArg()) * 86400;
		goto readArgs;
	}
	if (argIs("/H"))
	{
		TimeoutSec = toValue64(nextArg()) * 3600;
		goto readArgs;
	}
	RootDir = makeFullPath(nextArg());

	cout("TimeoutSec: %I64u %.3f %.3f\n", TimeoutSec, TimeoutSec / 3600.0, TimeoutSec / 86400.0);
	cout("RootDir: %s\n", RootDir);

	errorCase(!m_isRange(TimeoutSec, 0, IMAX_64));
	errorCase(!existDir(RootDir));

	errorCase(_stricmp("DELETE-OLD-CACHE", nextArg())); // ���S�̂���

	InitFileInfos();

	SlimFiles();
}
