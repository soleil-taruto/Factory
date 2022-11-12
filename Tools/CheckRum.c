/*
	.rum ディレクトリをチェックします。

	CheckRum.exe [/-S] [RUM_DIR]

		/-S ... files.txt, tree.txt の（中身の）パスを isFairRelPath() しない。
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\OpenSource\md5.h"

static int NoStrict;

static autoList_t *RevDirs;
static autoList_t *RevFiles;

/*
	チェック項目
		RevDirs, 重複が無いこと。
		RevDirs, 上から順に作成できること。
		RevFiles, 重複が無いこと。
		RevFiles, RevDirs のどれかの直下であること。
*/
static void CheckRevDirsRevFiles(void)
{
	uint i;
	uint j;

	for (i = 1; i < getCount(RevDirs); i++)
	for (j = 0; j < i; j++)
	{
		errorCase_m(!mbs_stricmp(getLine(RevDirs, i), getLine(RevDirs, j)), "ツリーファイルに重複があります。");
	}
	for (i = 0; i < getCount(RevDirs); i++)
	{
		char *dir = changeLocal(getLine(RevDirs, i), "");

		if (*dir) // ? ローカル名じゃない。
		{
			for (j = 0; j < i; j++)
				if (!mbs_stricmp(dir, getLine(RevDirs, j)))
					break;

			errorCase_m(j == i, "このツリーファイルは上から順に作成できません。");
		}
	}
	for (i = 1; i < getCount(RevFiles); i++)
	for (j = 0; j < i; j++)
	{
		errorCase_m(!mbs_stricmp(getLine(RevFiles, i), getLine(RevFiles, j)), "ファイルリストに重複があります。");
	}
	for (i = 1; i < getCount(RevFiles); i++)
	{
		char *dir = changeLocal(getLine(RevFiles, i), "");

		if (*dir) // ? ローカル名じゃない。
		{
			for (j = 0; j < getCount(RevDirs); j++)
				if (!mbs_stricmp(dir, getLine(RevDirs, j)))
					break;

			errorCase_m(j == getCount(RevDirs), "ファイルリストの問題：ツリーの配下に無いファイルがあります。");
		}
	}
}

static autoList_t *CheckAndReadLines(char *file, char *errorMessage)
{
	autoBlock_t *fileData = readBinary(file);
	uint index;
	char *text;
	autoList_t *lines;
	char *line;

	for (index = 0; index < getSize(fileData); index++)
	{
		int chr = getByte(fileData, index);

		errorCase_m(chr == '\0', errorMessage);
		errorCase_m(chr == '\n', errorMessage);

		if (chr == '\r')
		{
			index++;
			errorCase_m(getSize(fileData) <= index, errorMessage);
			chr = getByte(fileData, index);
			errorCase_m(chr != '\n', errorMessage);
		}
	}

	text = unbindBlock2Line(fileData);
#if 1
	removeChar(text, '\r');
#else // old, 遅い
	text = replaceLine(text, "\r\n", "\n", 0);
#endif

	lines = tokenize(text, '\n');
	memFree(text);

	errorCase_m(((char *)getLastElement(lines))[0], errorMessage); // 空のファイルか、改行で終わっているはず。

	memFree((char *)unaddElement(lines));

	foreach (lines, line, index)
	{
		errorCase_m(line[0] == '\0', errorMessage); // 空行は無いはず。
		errorCase_m(!isJLine(line, 1, 0, 0, 1), errorMessage);
	}
	return lines;
}
static void Check_IsCompactStamp(char *localDir)
{
	char *tmp = makeCompactStamp(getStampDataTime(compactStampToTime(localDir)));

	errorCase_m(strcmp(tmp, localDir), "日付に問題があります。");

	memFree(tmp);
}
static void Check_CommentTxt(void)
{
	autoList_t *lines = CheckAndReadLines("comment.txt", "コメントファイルが壊れています。");

	errorCase_m(getCount(lines) != 1, "コメントファイルの行数に問題があります。");

	releaseDim(lines, 1);
}
static void Check_FilesTxt(void)
{
	autoList_t *lines = CheckAndReadLines("files.txt", "ファイルリストが壊れています。");
	char *line;
	uint index;

	foreach (lines, line, index)
	{
		char *errorMessage = xcout("ファイルリストの %u 行目に問題があります。", index + 1);
		char *file;

		errorCase_m(!lineExp("<32,09AFaf> <>", line), errorMessage);
		file = line + 33;
		errorCase_m(!NoStrict && !isFairRelPath(file, 6), errorMessage); // 正しくても引っかかるかも？

		addElement(RevFiles, (uint)strx(file));

		memFree(errorMessage);
	}
	releaseDim(lines, 1);
}
static void Check_TreeTxt(void)
{
	autoList_t *lines = CheckAndReadLines("tree.txt", "ツリーファイルが壊れています。");
	char *line;
	uint index;

	foreach (lines, line, index)
	{
		char *errorMessage = xcout("ツリーファイルの %u 行目に問題があります。", index + 1);

		errorCase_m(!NoStrict && !isFairRelPath(line, 6), errorMessage); // 正しくても引っかかるかも？

		addElement(RevDirs, (uint)strx(line));

		memFree(errorMessage);
	}
	releaseDim(lines, 1);
}

static void CheckRum(char *rumDir)
{
	errorCase_m(m_isEmpty(rumDir), "指定されたパスに問題があります。");
	errorCase_m(!existDir(rumDir), "指定されたディレクトリは存在しません。");

	rumDir = makeFullPath(rumDir);

	errorCase_m(_stricmp(getExt(rumDir), "rum"), "指定されたパスの拡張子に問題があります。");
	errorCase(!existDir(rumDir)); // 2bs

	addCwd(rumDir);

	// ---- .rum 直下 ----

	errorCase_m(!existDir("files"), "files ディレクトリがありません。");
	errorCase_m(!existDir("revisions"), "revisions ディレクトリがありません。");

	{
		autoList_t *paths = ls(rumDir);

		errorCase_m(getCount(paths) != 2, "余計なファイルまたはディレクトリがあります。");

		releaseDim(paths, 1);
	}

	// ---- files ----

	LOGPOS();

	{
		autoList_t *files = ls("files");
		char *file;
		uint index;

		sortJLinesICase(files);

		foreach (files, file, index)
		{
			char *localFile;
			char *md5;

			cout("%s\n", file);

			errorCase_m(!existFile(file), "ファイルが見つかりません。");

			localFile = getLocal(file);
			md5 = md5_makeHexHashFile(file);

			cout("%s\n", localFile);
			cout("%s\n", md5);

			errorCase_m(!lineExp("<32,09AFaf>", localFile), "ファイル名がフォーマットに一致しません。");
			errorCase_m(_stricmp(localFile, md5), "ファイルが破損しています。");
		}
		releaseDim(files, 1);
	}

	// ---- revisions ----

	LOGPOS();

	{
		autoList_t *dirs = ls("revisions");
		char *dir;
		uint index;

		sortJLinesICase(dirs);

		foreach (dirs, dir, index)
		{
			char *localDir;

			cout("%s\n", dir);

			errorCase_m(!existDir(dir), "ディレクトリが見つかりません。");

			localDir = getLocal(dir);

			cout("%s\n", localDir);

			errorCase_m(!lineExp("<14,09>", localDir), "ディレクトリ名がフォーマットに一致しません。");
			Check_IsCompactStamp(localDir);

			// ---- revision ----

			addCwd(dir);

			errorCase_m(!existFile("comment.txt"), "コメントファイルが見つかりません。");
			errorCase_m(!existFile("files.txt"), "ファイルリストが見つかりません。");
			errorCase_m(!existFile("tree.txt"), "ツリーファイルが見つかりません。");

			{
				autoList_t *paths = ls(".");

				errorCase_m(getCount(paths) != 3, "余計なファイルまたはディレクトリがあります。");

				releaseDim(paths, 1);
			}

			RevDirs = newList();
			RevFiles = newList();

			Check_CommentTxt();
			Check_FilesTxt();
			Check_TreeTxt();

			CheckRevDirsRevFiles();

			releaseDim(RevDirs, 1);
			releaseDim(RevFiles, 1);

			unaddCwd();

			// ----
		}
	}

	// ----

	unaddCwd();

	cout("%s\n", rumDir);
	cout("+----------------------------+\n");
	cout("| 問題は見つかりませんでした |\n");
	cout("+----------------------------+\n");

	memFree(rumDir);
}

int main(int argc, char **argv)
{
readArgs:
	if (argIs("/-S"))
	{
		NoStrict = 1;
		goto readArgs;
	}
	if (hasArgs(1))
	{
		CheckRum(nextArg());
termination(0); // test
		return;
	}

	for (; ; )
	{
		CheckRum(c_dropDir());
		cout("\n");
	}
}
