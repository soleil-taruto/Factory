/*
	dmd5.exe [/I] [/-SVN] ...

		/I    ... ツリー比較時にパスの大文字／小文字を無視
		/-SVN ... サブバージョンを無視

	dmd5.exe ... /C

		LOOP { compare dropDir, dropDir }

	dmd5.exe ... /D

		LOOP { dropDir -> disp md5 + check CRED_FILE(DIR + .cred) }

	dmd5.exe ... /O DIR CRED_FILE

		DIR -> disp md5 + make CRED_FILE

		★ CRED_FILE が既に存在した場合は上書き確認する。

	dmd5.exe ... /O DIR

		DIR -> disp md5 + make CRED_FILE(DIR + .cred)

		★ CRED_FILE が既に存在した場合は上書き確認する。

	dmd5.exe ... /O

		LOOP { dropDir -> disp md5 + make CRED_FILE(DIR + .cred) }

		★ CRED_FILE が既に存在した場合は上書き確認する。

	dmd5.exe ... /P DIR CRED_FILE

		DIR -> disp md5 + check CRED_FILE

	dmd5.exe ... /S DIR

		DIR -> disp(Simple) md5 + check CRED_FILE(DIR + .cred)

		★表示されるハッシュ値は dmd5 DIR > out.txt ⇒ md5 out.txt で表示される値と同じ。

	dmd5.exe ... /S

		CWD -> disp(Simple) md5 + check CRED_FILE(CWD + .cred)

		★表示されるハッシュ値は dmd5 . > out.txt ⇒ md5 out.txt で表示される値と同じ。

	dmd5.exe ... DIR1 DIR2

		compare DIR1, DIR2

	dmd5.exe ... DIR

		DIR -> disp md5 + check CRED_FILE(DIR + .cred)

	dmd5.exe ...

		CWD -> disp md5 + check CRED_FILE(CWD + .cred)
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\OpenSource\md5.h"

#define EXT_CREDENTIALS "cred"

static int IgnoreCaseOfPath;

static autoList_t *GetHashes(char *targetDir)
{
	autoList_t *paths = lss(targetDir);
	autoList_t dirs;
	autoList_t files;
	autoList_t *hashes = newList();
	char *dir;
	char *file;
	uint index;

	targetDir = makeFullPath(targetDir);
	changeRoots(paths, targetDir, NULL);
	addCwd(targetDir);

	dirs = gndSubElements(paths, 0, lastDirCount);
	files = gndFollowElements(paths, lastDirCount);

	sortJLinesICase(&dirs);
	sortJLinesICase(&files);

	foreach (&dirs, dir, index)
	{
		addElement(hashes, (uint)xcout("-------------------------------- %s", dir));
	}
	foreach (&files, file, index)
	{
		cmdTitle_x(xcout("dmd5 - %u / %u", index, getCount(&files)));
		addElement(hashes, (uint)xcout("%s %s", c_md5_makeHexHashFile(file), file));
	}
	cmdTitle("dmd5");
	unaddCwd();
	memFree(targetDir);

	return hashes;
}
static void DispHashesDiff(autoList_t *hashes1, autoList_t *hashes2)
{
	if (isSameLines(hashes1, hashes2, IgnoreCaseOfPath))
	{
		cout("+--------------+\n");
		cout("| 一致しました |\n");
		cout("+--------------+\n");
	}
	else
	{
		autoList_t *report = getDiffLinesReportLim(hashes1, hashes2, IgnoreCaseOfPath, 20);
		char *line;
		uint index;

		cout("+--------------------+\n");
		cout("| 一致しませんでした |\n");
		cout("+--------------------+\n");

		foreach (report, line, index)
		{
			cout("%s\n", line);
		}
		releaseDim(report, 1);
	}
}

static int DispHashes_SimpleMode;

static void DispHashes(autoList_t *hashes)
{
	char *hash;
	uint index;

	if (DispHashes_SimpleMode)
	{
		md5_t *i = md5_create();
		autoBlock_t gab;

		foreach (hashes, hash, index)
		{
			md5_update(i, gndBlockLineVarPtr(hash, &gab));
			md5_update(i, gndBlockLineVarPtr("\r\n", &gab));
		}
		cout("%s\n", c_makeHexLine_x(md5_makeHash(i)));
		md5_release(i);
	}
	else
	{
		foreach (hashes, hash, index)
		{
			cout("%s\n", hash);
		}
	}
}

static void CompareDir(char *dir1, char *dir2)
{
	autoList_t *hashes1 = GetHashes(dir1);
	autoList_t *hashes2 = GetHashes(dir2);

	DispHashesDiff(hashes1, hashes2);

	releaseDim(hashes1, 1);
	releaseDim(hashes2, 1);
}
static void CheckDirCredFile(char *dir, char *credFile)
{
	autoList_t *hashes = GetHashes(dir);

	DispHashes(hashes);

	if (existFile(credFile))
	{
		autoList_t *credHashes = readLines(credFile);

		DispHashesDiff(hashes, credHashes);
		releaseDim(credHashes, 1);
	}
	releaseDim(hashes, 1);
}
static void CheckDir(char *dir)
{
	char *credFile = addExt(makeFullPath(dir), EXT_CREDENTIALS);

	CheckDirCredFile(dir, credFile);
	memFree(credFile);
}
static void MakeCredentialsCredFile(char *dir, char *credFile)
{
	autoList_t *hashes = GetHashes(dir);

	DispHashes(hashes);

	if (existFile(credFile))
	{
		autoList_t *credHashes = readLines(credFile);

//		DispHashesDiff(hashes, credHashes); // del @ 2018.3.14
		releaseDim(credHashes, 1);

		cout("上書き？\n");

		if (clearGetKey() == 0x1b)
			goto endFunc;

		cout("\n");
	}
	writeLines(credFile, hashes);

	cout("+--------------------------------+\n");
	cout("| ハッシュリストを書き出しました |\n");
	cout("+--------------------------------+\n");

endFunc:
	releaseDim(hashes, 1);
}
static void MakeCredentials(char *dir)
{
	char *credFile = addExt(makeFullPath(dir), EXT_CREDENTIALS);

	MakeCredentialsCredFile(dir, credFile);
	memFree(credFile);
}

int main(int argc, char **argv)
{
readArgs:
	if (argIs("/I")) // Ignore case of path
	{
		IgnoreCaseOfPath = 1;
		goto readArgs;
	}
	if (argIs("/-SVN")) // Ignore .svn etc.
	{
		antiSubversion = 1;
		goto readArgs;
	}

	if (argIs("/C")) // drop and Compare
	{
		for (; ; )
		{
			char *dir1;
			char *dir2;

			dir1 = dropDir();
			dir2 = dropDir();

			CompareDir(dir1, dir2);
			cout("\n");
		}
	}
	if (argIs("/D")) // Drop
	{
		for (; ; )
		{
			CheckDir(dropDir());
			cout("\n");
		}
	}
	if (argIs("/O")) // Output credentials file
	{
		if (hasArgs(2))
		{
			MakeCredentialsCredFile(getArg(0), getArg(1));
			return;
		}
		if (hasArgs(1))
		{
			MakeCredentials(nextArg());
			return;
		}
		for (; ; )
		{
			MakeCredentials(c_dropDir());
			cout("\n");
		}
	}
	if (argIs("/P")) // comPare credentials file
	{
		CheckDirCredFile(getArg(0), getArg(1));
		return;
	}
	if (argIs("/S")) // Simple
	{
		DispHashes_SimpleMode = 1;

		if (hasArgs(1))
		{
			CheckDir(nextArg());
			return;
		}
		CheckDir(".");
		return;
	}

	if (hasArgs(2))
	{
		char *dir1;
		char *dir2;

		dir1 = nextArg();
		dir2 = nextArg();

		CompareDir(dir1, dir2);
		return;
	}
	if (hasArgs(1))
	{
		CheckDir(nextArg());
		return;
	}
	CheckDir(".");
}
