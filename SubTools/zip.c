/*
	zip.exe /P ZIP-FILE SRC-DIR [BASE-NAME]

		... SRC-DIR を ZIP-FILE にパックする。
			不要な最上位階層を除去する。
			BASE-NAME を指定すると、この名前をパック内の最上位階層にする。"$" を指定すると ZIP-FILE のローカル名のノードになる。
			ZIP-FILE の拡張子は "zip" でなくても良い。

	zip.exe /PK ZIP-FILE SRC-DIR [BASE-NAME]

		... SRC-DIR を ZIP-FILE にパックする。
			最上位階層はそのまま！ (/Pとの違いはここだけ！)
			BASE-NAME を指定すると、この名前をパック内の最上位階層にする。"$" を指定すると ZIP-FILE のローカル名のノードになる。
			ZIP-FILE の拡張子は "zip" でなくても良い。

	zip.exe /R ZIP-FILE [BASE-NAME]

		... ZIP-FILE を再パックする。
			不要な最上位階層を除去する。
			BASE-NAME を指定すると、この名前をパック内の最上位階層にする。"$" を指定すると ZIP-FILE のローカル名のノードになる。
			ZIP-FILE の拡張子は "zip" でなくても良い。

	zip.exe /RB ZIP-FILE

		... ZIP-FILE を再パックする。
			不要な最上位階層を除去する。
			ZIP-FILE のローカル名の拡張子を除去した名前をパック内の最上位階層にする。
			ZIP-FILE の拡張子は "zip" でなくても良い。

	zip.exe /RBD ROOT-DIR

		... ROOT-DIR 配下の全ての「拡張子 "zip" のファイル」について
			> zip.exe /RB ZIP-FILE
			と同じ処理を行う。

	zip.exe [/PE-] /O OUT-DIR PROJ-NAME

		... OUT-DIR -> OUT-DIR \ { PROJ-NAME } .zip

	zip.exe [/PE-] [/RVE-] [/B | /V VER-VAL] /G OUT-DIR PROJ-NAME

		... OUT-DIR -> OUT-DIR \ { PROJ-NAME } _v123.zip

	zip.exe /X ZIP-FILE OUT-DIR

		... ZIP-FILE を OUT-DIR に展開する。
			OUT-DIR が存在しなければ作成する。
			最上位階層はそのまま！
			ZIP-FILE の拡張子は "zip" でなくても良い。
			OUT-DIR に既存のファイルがあったら上書きする。
			OUT-DIR の既存のファイルに対してディレクトリの上書きは失敗する。error(); にならない。
			OUT-DIR の既存のディレクトリに対してファイルの上書きは失敗する。error(); にならない。

	zip.exe /U ZIP-FILE OUT-DIR

		... /X と同じだが、こちらは...
			不要な最上位階層を除去する！
			OUT-DIR の既存のファイルに対してディレクトリの上書きは成功する。
			OUT-DIR の既存のディレクトリに対してファイルの上書きは成功する。

	----
	環境変数

	zip_NoPause

		... "1" のとき入力待ちを行わない。
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\CRandom.h"
#include "C:\Factory\OpenSource\md5.h"
#include "C:\Factory\Meteor\7z.h"
#include "libs\ChangePETimeDateStamp.h"

#define ZIP_MD5_HISTORY_FILE "C:\\Factory\\tmp\\zip_md5_history.txt"
#define ZIP_MD5_HISTORY_LMT 40

/*
	7-Zip Extra 16.02 か何か
	空白を含まないパスであること。
*/
#define ZIP7_LOCAL_FILE LOCALFILE_7Z_EXE
#define ZIP7_FILE FILE_7Z_EXE

#define VER_BETA 1100000000

#define BASENAME_AUTO "$"

static int IsBatchMode(void)
{
	return !strcmp("1", getEnvLine("zip_NoPause"));
}
static char *GetZip7File(void)
{
	static char *file;

	if(!file)
	{
		file = ZIP7_LOCAL_FILE;

		if(!existFile(file))
		{
			file = ZIP7_FILE;
			errorCase_m(!existFile(file), "[" ZIP7_FILE "]が見つかりません。");
		}
	}
	return file;
}
static char *IntoIfOneDir(char *dir)
{
	cout("dir1: %s\n", dir);

	for(; ; )
	{
		autoList_t *paths = ls(dir);

		if(getCount(paths) != 1 || !existDir(getLine(paths, 0)))
		{
			releaseDim(paths, 1);
			break;
		}
		dir = strz(dir, getLine(paths, 0));
		releaseDim(paths, 1);
	}
	cout("dir2: %s\n", dir);
	return dir;
}
static void ExtractZipFile(char *zipFile, char *destDir)
{
	coExecute_x(xcout("%s x -y \"%s\" -o\"%s\"", GetZip7File(), zipFile, destDir));
}

static char *PZF_DestZipFile;

static void PackZipFile(char *zipFile, char *srcDir)
{
	char *md5;

	removeFileIfExist(zipFile);

	addCwd(srcDir);
	coExecute_x(xcout("%s a \"%s\" *", GetZip7File(), zipFile));
	unaddCwd();

	cout("+--- zip md5 ----------------------+\n");
	cout("| %s |\n", md5 = md5_makeHexHashFile(zipFile));
	cout("+----------------------------------+\n");

	if(isFactoryDirEnabled()) // history
	{
		autoList_t *lines = existFile(ZIP_MD5_HISTORY_FILE) ?
			readLines(ZIP_MD5_HISTORY_FILE) :
			newList();

		while(ZIP_MD5_HISTORY_LMT < getCount(lines))
			memFree((char *)desertElement(lines, 0));

		addElement(lines, (uint)xcout("----"));
		addElement(lines, (uint)xcout("[%s]", c_makeJStamp(NULL, 0)));
		addElement(lines, (uint)xcout("< %s", PZF_DestZipFile ? PZF_DestZipFile : zipFile));
		addElement(lines, (uint)xcout("%s", md5));

		writeLines(ZIP_MD5_HISTORY_FILE, lines);
		releaseDim(lines, 1);
	}
	memFree(md5);
}
static uint InputVersion(void) // ret: 0 == cancel, 1 ～ 999 == "0.01" ～ "9.99", VER_BETA == BETA
{
	char *sVersion;
	uint version;

	if(IsBatchMode())
	{
		LOGPOS();
		return VER_BETA;
	}
	cout("######################################################################\n");
	cout("## バージョン番号を入力して下さい。[1-999] as 0.01-9.99, [] as BETA ##\n");
	cout("######################################################################\n");

	sVersion = coInputLine();

	if(*sVersion)
	{
		version = toValue(sVersion);
		m_range(version, 0, 999);
	}
	else
		version = coil_esc ? 0 : VER_BETA;

	memFree(sVersion);
	return version;
}
static uint FindStringInExe(autoBlock_t *text, char *ptnString) // ret: UINTMAX == not found
{
	uint index;
	uint rPos = 0;

	for(index = 0; index < getSize(text); index++)
	{
	recheck:
		if(getByte(text, index) == ptnString[rPos]) // 最後の '\0' まで一致するか判定する。
		{
			if(ptnString[rPos] == '\0') // ? found
				return index - rPos;

			rPos++;
		}
		else if(rPos)
		{
			rPos = 0;
			goto recheck;
		}
	}
	return UINTMAX; // not found
}
static void DestroyFindVersionPtn(autoBlock_t *text, uint startPos, uint count)
{
	uint index;

	for(index = 0; index < count; index++)
	{
		int chr;

#if 1
		chr = "Coffee"[index % 6];
#elif 1
		do
		{
			chr = '\x21' + getCryptoByte() % ('\x7e' + 1 - '\x21');
		}
		while(chr == ':');
#else
		chr = ';'; // ':' でなければ何でも良い。
#endif

		setByte(text, startPos + index, chr);
	}
}

static int ReplaceVersionExeFileDisabled;

static void ReplaceVersion(char *dir, uint version) // version: 1 ～ 999, VER_BETA
{
	autoList_t *files = lssFiles(dir);
	char *file;
	uint index;
	char *sVersion;

	if(version == VER_BETA)
		sVersion = strx("BETA");
	else
		sVersion = xcout("%u.%02u", version / 100, version % 100);

	foreach(files, file, index)
	{
		if(
			!_stricmp("Manual.txt", getLocal(file)) ||
			!_stricmp("マニュアル.txt", getLocal(file)) ||
			!_stricmp("Properties.dat", getLocal(file))
			)
//		if(!_stricmp("txt", getExt(file))) // old @ 2017.8.5
		{
			char *text = readText(file);
			char *newText;

			newText = strx(text);
			newText = replaceLine(newText, "$version$", sVersion, 1);

			LOGPOS();

			if(strcmp(text, newText)) // ? text != newText
			{
				LOGPOS();
				writeOneLineNoRet(file, newText);
			}
			memFree(text);
			memFree(newText);
		}
		else if(!ReplaceVersionExeFileDisabled && !_stricmp("exe", getExt(file)))
		{
			static char *CONCERT_PTN = "{a9a54906-791d-4e1a-8a71-a4c69359cf68}:0.00"; // shared_uuid@g
			autoBlock_t *text = readBinary(file);
			uint conPos;

			conPos = FindStringInExe(text, CONCERT_PTN);

			LOGPOS();

			if(conPos != UINTMAX)
			{
				LOGPOS();
				errorCase(strlen(sVersion) != 4); // 2bs
				strcpy((char *)directGetBuffer(text) + conPos + strlen(CONCERT_PTN) - 4, sVersion);
				errorCase(FindStringInExe(text, CONCERT_PTN) != UINTMAX); // ? 2箇所以上ある
				DestroyFindVersionPtn(text, conPos, strlen(CONCERT_PTN) - 5);
				writeBinary(file, text);
			}
			releaseAutoBlock(text);
		}
	}
	releaseDim(files, 1);
	memFree(sVersion);
}
static char *MakeRev(void)
{
	char *revFile = makeTempPath(NULL);
	char *rev;

	coExecute_x(xcout("C:\\Factory\\DevTools\\rev.exe //O \"%s\" /P", revFile));

	rev = readFirstLine(revFile);
	errorCase(!lineExp("<4,09>.<3,09>.<5,09>", rev)); // 2bs
	replaceChar(rev, '.', '0');
	errorCase(!lineExp("<14,09>", rev)); // 2bs
	removeFile_x(revFile);
	return rev;
}
static char *c_MakeRev(void)
{
	static char *stock;
	memFree(stock);
	return stock = MakeRev();
}
static char *GetPathTailVer(uint version) // ret: bind
{
	static char *pathTail;

	memFree(pathTail);

	if(version == VER_BETA)
		pathTail = xcout("_BETA_%s", c_MakeRev());
	else
		pathTail = xcout("_v%03u", version);

	return pathTail;
}
static void PackZipFileEx_K1D(char *zipFile, char *srcDir, int srcDirRmFlag, char *baseName, uint version, int keepOneDir)
{
	char *workDir = makeTempDir(NULL);
	char *destDir;

	srcDir = strx(srcDir);

	if(!keepOneDir)
		srcDir = IntoIfOneDir(srcDir);

	destDir = strx(workDir);

	if(baseName)
	{
		char *tmpbn;

		if(!strcmp(baseName, BASENAME_AUTO))
		{
			tmpbn = changeExt(getLocal(zipFile), "");
			cout("baseName: %s -> %s\n", BASENAME_AUTO, tmpbn);
		}
		else
			tmpbn = strx(baseName);

		destDir = combine_xc(destDir, tmpbn);
		memFree(tmpbn);

		if(version)
			destDir = addLine(destDir, GetPathTailVer(version));

		createDir(destDir);
	}
	if(srcDirRmFlag)
	{
		moveDir(srcDir, destDir);
		removeDir(srcDir);
	}
	else
		copyDir(srcDir, destDir);

	if(version)
		ReplaceVersion(destDir, version);

	PackZipFile(zipFile, workDir);

	recurRemoveDir(workDir);

	memFree(srcDir);
	memFree(workDir);
	memFree(destDir);
}
static void PackZipFileEx(char *zipFile, char *srcDir, int srcDirRmFlag, char *baseName, uint version)
{
	PackZipFileEx_K1D(zipFile, srcDir, srcDirRmFlag, baseName, version, 0);
}
static void RepackZipFile(char *zipFile, char *baseName)
{
	char *workDir = makeTempDir(NULL);

	ExtractZipFile(zipFile, workDir);
	PackZipFileEx(zipFile, workDir, 1, baseName, 0);

	memFree(workDir);
}
static void RepackAllZipFile(char *rootDir)
{
	autoList_t *files = lssFiles(rootDir);
	char *file;
	uint index;

	foreach(files, file, index)
	{
		if(!_stricmp("ZIP", getExt(file)))
		{
			char *baseName = getLocal(file);

			baseName = changeExt(baseName, "");
			baseName = lineToFairLocalPath_x(baseName, 100);

			cout("file: %s\n", file);
			cout("baseName: %s\n", baseName);

			RepackZipFile(file, baseName);

			memFree(baseName);
		}
	}
	releaseDim(files, 1);
}

static int ChangePEDisabled;

static void AdjustAllPETimeDateStamp(char *dir)
{
	if(!ChangePEDisabled)
		ChangeAllPETimeDateStamp(dir, 0x5aaaaaaa);
}
int main(int argc, char **argv)
{
	uint autoVersion = 0;

	errorCase_m(!existFile(ZIP7_LOCAL_FILE) && !existFile(ZIP7_FILE), "7zさんが居ません。");

readArgs:
	if(argIs("/PE-"))
	{
		LOGPOS();
		ChangePEDisabled = 1;
		goto readArgs;
	}
	if(argIs("/RVE-"))
	{
		LOGPOS();
		ReplaceVersionExeFileDisabled = 1;
		goto readArgs;
	}
	if(argIs("/B"))
	{
		autoVersion = VER_BETA;
		goto readArgs;
	}
	if(argIs("/V"))
	{
		autoVersion = toValue(nextArg());
		errorCase(!m_isRange(autoVersion, 1, 999));
		goto readArgs;
	}

	/*
		★★★ パスはこの関数内でフルパスにすること。★★★
	*/

	if(argIs("/P"))
	{
		char *zipFile;
		char *srcDir;
		char *baseName;

		zipFile = makeFullPath(nextArg());
		srcDir  = makeFullPath(nextArg());
		baseName = hasArgs(1) ? lineToFairLocalPath(nextArg(), 100) : NULL;

		cout("[Pack]\n");
		cout("zipFile: %s\n", zipFile);
		cout("srcDir: %s\n", srcDir);
		cout("baseName: %s\n", baseName ? baseName : "<none>");

		PackZipFileEx(zipFile, srcDir, 0, baseName, 0);

		memFree(zipFile);
		memFree(srcDir);
		memFree(baseName);
		return;
	}
	if(argIs("/PK"))
	{
		char *zipFile;
		char *srcDir;
		char *baseName;

		zipFile = makeFullPath(nextArg());
		srcDir  = makeFullPath(nextArg());
		baseName = hasArgs(1) ? lineToFairLocalPath(nextArg(), 100) : NULL;

		cout("[PACK]\n");
		cout("zipFile: %s\n", zipFile);
		cout("srcDir: %s\n", srcDir);
		cout("baseName: %s\n", baseName ? baseName : "<none>");

		PackZipFileEx_K1D(zipFile, srcDir, 0, baseName, 0, 1);

		memFree(zipFile);
		memFree(srcDir);
		memFree(baseName);
		return;
	}
	if(argIs("/R"))
	{
		char *zipFile;
		char *baseName;

		zipFile = makeFullPath(nextArg());
		baseName = hasArgs(1) ? lineToFairLocalPath(nextArg(), 100) : NULL;

		cout("[Repack]\n");
		cout("zipFile: %s\n", zipFile);
		cout("baseName: %s\n", baseName ? baseName : "<none>");

		RepackZipFile(zipFile, baseName);

		memFree(zipFile);
		memFree(baseName);
		return;
	}
	if(argIs("/RB"))
	{
		char *zipFile;
		char *baseName;

		zipFile = makeFullPath(nextArg());
		baseName = getLocal(zipFile);
		baseName = changeExt(baseName, "");
		baseName = lineToFairLocalPath_x(baseName, 100);

		cout("[Repack(Base)]\n");
		cout("zipFile: %s\n", zipFile);
		cout("baseName: %s\n", baseName);

		RepackZipFile(zipFile, baseName);

		memFree(zipFile);
		memFree(baseName);
		return;
	}
	if(argIs("/RBD"))
	{
		char *rootDir = makeFullPath(nextArg());

		cout("[Repack(Base+RootDir)]\n");
		cout("rootDir: %s\n", rootDir);

		RepackAllZipFile(rootDir);

		memFree(rootDir);
		return;
	}
	if(argIs("/O")) // C:\Dev のリリース向け
	{
		char *outDir;
		char *projName;
		char *destZipFile;
		char *midZipFile;

		outDir = makeFullPath(nextArg());
		projName = lineToFairLocalPath(nextArg(), 100);
		destZipFile = combine_cx(outDir, addExt(strx(projName), "zip"));
		midZipFile = makeTempPath("zip");

		cout("[Pack]\n");
		cout("outDir: %s\n", outDir);
		cout("projName: %s\n", projName);
		cout("destZipFile: %s\n", destZipFile);
		cout("midZipFile: %s\n", midZipFile);

		AdjustAllPETimeDateStamp(outDir);
		PZF_DestZipFile = destZipFile;
		PackZipFileEx(midZipFile, outDir, 1, projName, 0);
		PZF_DestZipFile = NULL;
		createDir(outDir);
		coExecute_x(xcout("Compact.exe /C \"%s\"", outDir)); // 圧縮
		copyFile(midZipFile, destZipFile);
		removeFile(midZipFile);

		memFree(outDir);
		memFree(projName);
		memFree(destZipFile);
		memFree(midZipFile);
		return;
	}
	if(argIs("/G")) // C:\Dev のゲーム（バージョン番号付きアプリ）のリリース向け
	{
		char *outDir;
		char *projName;
		char *destZipFile;
		char *midZipFile;
		uint version = autoVersion ? autoVersion : InputVersion();

		outDir = makeFullPath(nextArg());
		projName = lineToFairLocalPath(nextArg(), 100);
		destZipFile = combine_cx(outDir, addExt(xcout("%s%s", projName, GetPathTailVer(version)), "zip"));
		midZipFile = makeTempPath("zip");

		if(version)
		{
			cout("[Pack(Game)]\n");
			cout("outDir: %s\n", outDir);
			cout("projName: %s\n", projName);
			cout("destZipFile: %s\n", destZipFile);
			cout("midZipFile: %s\n", midZipFile);
			cout("version: %03u\n", version);

			AdjustAllPETimeDateStamp(outDir);
			PZF_DestZipFile = destZipFile;
			PackZipFileEx(midZipFile, outDir, 1, projName, version);
			PZF_DestZipFile = NULL;
			createDir(outDir);
			coExecute_x(xcout("Compact.exe /C \"%s\"", outDir)); // 圧縮
			copyFile(midZipFile, destZipFile);
			removeFile(midZipFile);
		}
		else
		{
			cout("キャンセルされました。\n");

#if 0
			recurClearDir(outDir);
#else
			semiRemovePath(outDir);
			createDir(outDir);
#endif
		}
		memFree(outDir);
		memFree(projName);
		memFree(destZipFile);
		memFree(midZipFile);
		return;
	}
	if(argIs("/X"))
	{
		char *zipFile;
		char *outDir;

		zipFile = makeFullPath(nextArg());
		outDir  = makeFullPath(nextArg());

		errorCase(!existFile(zipFile));

		if(!existDir(outDir))
			createPath(outDir, 'D');

		ExtractZipFile(zipFile, outDir);

		memFree(zipFile);
		memFree(outDir);
		return;
	}
	if(argIs("/U"))
	{
		char *zipFile;
		char *outDir;
		char *midDir;
		char *midDirBk;

		zipFile = makeFullPath(nextArg());
		outDir  = makeFullPath(nextArg());

		errorCase(!existFile(zipFile));

		if(!existDir(outDir))
			createPath(outDir, 'D');

		midDir = makeFreeDir();
		midDirBk = strx(midDir);

		ExtractZipFile(zipFile, midDir);
		midDir = IntoIfOneDir(midDir);
		moveDir(midDir, outDir);
		recurRemoveDir(midDirBk);

		memFree(zipFile);
		memFree(outDir);
		memFree(midDir);
		memFree(midDirBk);
		return;
	}
}
