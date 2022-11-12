/*
	MatomeMaker.exe C:\Dev\subdir
*/

#include "C:\Factory\Common\all.h"

#define FILE_SIZE_LIMIT 1050000 // 1048576 == 1024 * 1024

static char *RDir;

#define W_DIR "C:\\temp\\Matome"
//#define LASTEXEC_DATETIME_FILE "C:\\appdata\\MatomeMaker_LastExecDateTime.txt"

static int IsCollectExt(char *ext)
{
	return

		// extra >

		!_stricmp(ext, "TPF") ||

		// < extra

		!_stricmp(ext, "C") ||
		!_stricmp(ext, "H") ||
		!_stricmp(ext, "CS") ||
		!_stricmp(ext, "CPP") ||
		!_stricmp(ext, "BAT") ||
		!_stricmp(ext, "TXT");
}
static int IsCorrect(char *entFile, char *relFile)
{
	return IsCollectExt(getExt(relFile)) && getFileSize(entFile) < FILE_SIZE_LIMIT;
}
static void TrmRum(char *rumDir)
{
	char *entsDir = combine(rumDir, "files");
	char *revsDir = combine(rumDir, "revisions");
	autoList_t *revs;
	char *rev;
	uint rev_index;

	errorCase(!existDir(rumDir));
	errorCase(!existDir(entsDir));
	errorCase(!existDir(revsDir));

#if 0
	// 既知のリビジョンを除外
	{
		char *lastExecDateTime;

		if (existFile(LASTEXEC_DATETIME_FILE))
			lastExecDateTime = readFirstLine(LASTEXEC_DATETIME_FILE);
		else
			lastExecDateTime = strx("00000000000000");
			//                       YYYYMMDDhhmmss

		revs = lsDirs(revsDir);

		foreach (revs, rev, rev_index)
		{
			if (strcmp(getLocal(rev), lastExecDateTime) < 0) // ? rev < lastExecDateTime
			{
				cout("K %s\n", rev);
				recurRemoveDir(rev);
			}
		}
		releaseDim(revs, 1);
		memFree(lastExecDateTime);

		if (!lsCount(revsDir)) // リビジョンが無くなったら .rum ごと捨てる。
		{
			cout("E %s\n", rumDir);
			recurRemoveDir(rumDir);
			goto endFunc;
		}
	}
#endif

	revs = lsDirs(revsDir);

	foreach (revs, rev, rev_index)
	{
		char *filesFile = combine(rev, "files.txt");
		autoList_t *lines;
		char *line;
		uint line_index;

		errorCase(!existFile(filesFile));

		lines = readLines(filesFile);

		foreach (lines, line, line_index)
		{
			char *entFile;
			char *relFile;

			errorCase(strlen(line) < 34); // 最短 -> "0123456789abcdef0123456789abcdef x"
			errorCase(line[32] != ' ');

			line[32] = '\0';

			entFile = combine(entsDir, line);
			relFile = strx(line + 33);

			line[32] = ' ';

			errorCase_m(!existFile(entFile), "need rum /t");

			if (!IsCorrect(entFile, relFile))
				line[0] = '\0';

			memFree(entFile);
			memFree(relFile);
		}
		trimLines(lines);

		writeLines(filesFile, lines);

		memFree(filesFile);
		releaseDim(lines, 1);
	}
	releaseDim(revs, 1);

	coExecute_x(xcout("C:\\Factory\\Tools\\rum.exe /TT \"%s\"", rumDir));

endFunc:
	memFree(entsDir);
	memFree(revsDir);
}
static void CpRums(void)
{
	autoList_t *dirs = lssDirs(RDir);
	char *dir;
	uint index;

	LOGPOS();

	foreach (dirs, dir, index)
	{
		if (!_stricmp("rum", getExt(dir)))
		{
			char *destDir = changeRoot(strx(dir), RDir, W_DIR);

			cout("< %s\n", dir);
			cout("> %s\n", destDir);

			LOGPOS();
			createPath(destDir, 'D');
			LOGPOS();
			copyDir(dir, destDir);
			LOGPOS();
			TrmRum(destDir);
			LOGPOS();

			memFree(destDir);
		}
	}
	releaseDim(dirs, 1);

	LOGPOS();
}
int main(int argc, char **argv)
{
	RDir = nextArg();
	RDir = makeFullPath(RDir);

	errorCase(!existDir(RDir));

	recurRemoveDirIfExist(W_DIR);
	createDir(W_DIR);

#if 0
	if (existFile(LASTEXEC_DATETIME_FILE)) // 前回の実行日時を確認
	{
		char *lastExecDateTime = readFirstLine(LASTEXEC_DATETIME_FILE);
		char *jStamp;

		jStamp = makeJStamp(getStampDataTime(compactStampToTime(lastExecDateTime)), 0);

		cout("前回の実行日時は %s (%s) です。\n", jStamp, lastExecDateTime);
		cout("これより古いリビジョンは除外します。\n");
		cout("続行？\n");

		if (clearGetKey() == 0x1b)
			termination(0);

		cout("続行！\n");

		memFree(lastExecDateTime);
		memFree(jStamp);
	}
#endif

	CpRums();

#if 0
	// 今回の実行日時を保存
	{
		char *dateTime = makeCompactStamp(NULL);

		writeOneLine(LASTEXEC_DATETIME_FILE, dateTime);

		memFree(dateTime);
	}
#endif

	coExecute("START " W_DIR);
}
