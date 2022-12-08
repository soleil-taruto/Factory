/*
	差分は、指定されたフォルダの lsFiles() についてのみ処理する。
	差分フォルダ内の一つのファイルは、空きメモリの１／３より小さいこと。
*/

#include "Sabun.h"

#define FILEHDR_SIGNATURE "tartelette-Factory_Sabun_" __DATE__ __TIME__

static void DiffnomukouWoZurasu(autoBlock_t *fileImage1, autoBlock_t *fileImage2)
{
	SBN_ShiftBlock(fileImage2, fileImage1);
}
static void ZureWoModosu(autoBlock_t *fileImage)
{
	SBN_UnshiftBlock(fileImage);
}

static autoBlock_t *GetSabun(autoBlock_t *fileImage1, autoBlock_t *fileImage2)
{
	autoBlock_t *sabun = newBlock();
	uint index;

	for (index = 0; index < getSize(fileImage2); index++)
	{
		int chr1 = refByte(fileImage1, index);
		int chr2 = getByte(fileImage2, index);

		addByte(sabun, chr1 ^ chr2);
	}
	return sabun;
}
static void ApplySabun(autoBlock_t *fileImage, autoBlock_t *sabun)
{
	uint index;

	setSize(fileImage, getSize(sabun));

	for (index = 0; index < getSize(sabun); index++)
	{
		setByte(fileImage, index, getByte(fileImage, index) ^ getByte(sabun, index));
	}
}

static void CompressSabun(autoBlock_t *sabun)
{
	SBN_CompressBlock(sabun);
}
static void DecompressSabun(autoBlock_t *sabun)
{
	SBN_DecompressBlock(sabun);
}

void makeSabun(char *sabunFile, char *beforeDir, char *afterDir, int correctAddDelete)
{
	char *dir1 = beforeDir;
	char *dir2 = afterDir;
	autoList_t *files1;
	autoList_t *files2;
	autoList_t *mfiles;
	autoList_t *trailLines = newList();
	char *file;
	char *line;
	uint index;
	FILE *fp;

	// check no dirs.
	{
		autoList_t *dirs1 = lsDirs(dir1);
		autoList_t *dirs2 = lsDirs(dir2);

		errorCase(getCount(dirs1) || getCount(dirs2));

		releaseAutoList(dirs1);
		releaseAutoList(dirs2);
	}

	files1 = lsFiles(dir1);
	files2 = lsFiles(dir2);
	dir1 = makeFullPath(dir1);
	dir2 = makeFullPath(dir2);
	changeRoots(files1, dir1, NULL);
	changeRoots(files2, dir2, NULL);

	fp = fileOpen(sabunFile, "wb");

	writeLine(fp, FILEHDR_SIGNATURE);

	rapidSortLines(files1);
	rapidSortLines(files2);

	addCwd(dir1);
	foreach (files1, file, index) // 更新前のファイルとハッシュ、確認用
	{
		writeLine(fp, file);
		writeLine_x(fp, md5_makeHexHashFile(file));
	}
	writeChar(fp, '\n'); // 終了
	unaddCwd();

	addCwd(dir2);
	foreach (files2, file, index) // 更新後のファイルとハッシュ、確認用
	{
		addElement(trailLines, (uint)strx(file));
		addElement(trailLines, (uint)md5_makeHexHashFile(file));
	}
	unaddCwd();

	mfiles = merge(files1, files2, (sint (*)(uint, uint))mbs_stricmp, (void (*)(uint))memFree);

	if (!correctAddDelete)
	{
		if (getCount(files1) || getCount(files2))
		{
			fileClose(fp);
			removeFile(sabunFile);
			error();
		}
	}

	foreach (files1, file, index) // 削除するファイル
	{
		writeChar(fp, 'D');
		writeLine(fp, file);
	}
	addCwd(dir2);
	foreach (files2, file, index) // 追加するファイル
	{
		writeChar(fp, 'A');
		writeLine(fp, file);

		writeValue(fp, (uint)getFileSize(file));
		writeBinaryBlock_x(fp, readBinary(file));
	}
	unaddCwd();

	foreach (mfiles, file, index) // 更新するファイル
	{
		char *file1 = combine(dir1, file);
		char *file2 = combine(dir2, file);
		autoBlock_t *fileImage1;
		autoBlock_t *fileImage2;
		autoBlock_t *sabun;

		fileImage1 = readBinary(file1);
		fileImage2 = readBinary(file2);

		DiffnomukouWoZurasu(fileImage1, fileImage2);
		sabun = GetSabun(fileImage1, fileImage2);
		CompressSabun(sabun);

		writeChar(fp, 'U');
		writeLine(fp, file);

		writeValue(fp, getSize(sabun));
		writeBinaryBlock(fp, sabun);

		memFree(file1);
		memFree(file2);
		releaseAutoBlock(fileImage1);
		releaseAutoBlock(fileImage2);
		releaseAutoBlock(sabun);
	}
	writeChar(fp, 'Z'); // 終了

	foreach (trailLines, line, index)
	{
		writeLine(fp, line);
	}
	fileClose(fp);

	memFree(dir1);
	memFree(dir2);
	releaseDim(mfiles, 1);
	releaseDim(trailLines, 1);
}
int sabunUpdate(char *sabunFile, char *targetDir) // ret: ? アップデートした。対象外の場合 targetDir は一切変更せず、0 を返す。
{
	FILE *fp = fileOpen(sabunFile, "rb");
	int retval = 0;

	// Check Signature
	{
		char *fhdr = readLine(fp);

		errorCase(!fhdr);
		errorCase(strcmp(fhdr, FILEHDR_SIGNATURE));

		memFree(fhdr);
	}
	addCwd(targetDir);

	for (; ; ) // アップデート対象であるか確認
	{
		char *file = readLine(fp);
		char *hash;
		char *rHash;

		errorCase(!file);

		if (!*file) // ? 終了
		{
			memFree(file);
			break;
		}
		hash = readLine(fp);
		errorCase(!hash);

		rHash = existFile(file) ? md5_makeHexHashFile(file) : strx("*"); // ファイルが無い -> 常に不一致なハッシュ

		if (strcmp(hash, rHash)) // ? ハッシュ不一致
		{
			memFree(file);
			memFree(hash);
			memFree(rHash);
			goto endfunc;
		}
		memFree(file);
		memFree(hash);
		memFree(rHash);
	}
	retval = 1;

	for (; ; ) // アップデート処理
	{
		int chr = readChar(fp);
		char *file;

		if (chr == 'Z')
			break;

		file = readLine(fp);
		errorCase(!file);
		errorCase(!*file);

		if (chr == 'D') // 削除
		{
			removeFile(file);
		}
		else if (chr == 'A') // 追加
		{
			writeBinary_cx(file, readBinaryBlock(fp, readValue(fp)));
		}
		else if (chr == 'U') // 更新
		{
			autoBlock_t *fileImage = readBinary(file);
			autoBlock_t *sabun = readBinaryBlock(fp, readValue(fp));

			DecompressSabun(sabun);
			ApplySabun(fileImage, sabun);
			ZureWoModosu(fileImage);
			writeBinary(file, fileImage);

			releaseAutoBlock(fileImage);
			releaseAutoBlock(sabun);
		}
		else
		{
			error();
		}
		memFree(file);
	}

	for (; ; ) // アップデートが正しく適用されたか確認
	{
		char *file = readLine(fp);
		char *hash;
		char *rHash;

		if (!file) // ? 終了
			break;

		errorCase(!*file);
		errorCase(!existFile(file));

		hash = readLine(fp);
		errorCase(!hash);

		rHash = md5_makeHexHashFile(file);
		errorCase(strcmp(hash, rHash)); // ? ハッシュ不一致

		memFree(file);
		memFree(hash);
		memFree(rHash);
	}

endfunc:
	unaddCwd();
	fileClose(fp);

	return retval;
}
