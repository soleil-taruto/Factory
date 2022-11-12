/*
	.rum �f�B���N�g�����`�F�b�N���܂��B

	CheckRum.exe [/-S] [RUM_DIR]

		/-S ... files.txt, tree.txt �́i���g�́j�p�X�� isFairRelPath() ���Ȃ��B
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\OpenSource\md5.h"

static int NoStrict;

static autoList_t *RevDirs;
static autoList_t *RevFiles;

/*
	�`�F�b�N����
		RevDirs, �d�����������ƁB
		RevDirs, �ォ�珇�ɍ쐬�ł��邱�ƁB
		RevFiles, �d�����������ƁB
		RevFiles, RevDirs �̂ǂꂩ�̒����ł��邱�ƁB
*/
static void CheckRevDirsRevFiles(void)
{
	uint i;
	uint j;

	for (i = 1; i < getCount(RevDirs); i++)
	for (j = 0; j < i; j++)
	{
		errorCase_m(!mbs_stricmp(getLine(RevDirs, i), getLine(RevDirs, j)), "�c���[�t�@�C���ɏd��������܂��B");
	}
	for (i = 0; i < getCount(RevDirs); i++)
	{
		char *dir = changeLocal(getLine(RevDirs, i), "");

		if (*dir) // ? ���[�J��������Ȃ��B
		{
			for (j = 0; j < i; j++)
				if (!mbs_stricmp(dir, getLine(RevDirs, j)))
					break;

			errorCase_m(j == i, "���̃c���[�t�@�C���͏ォ�珇�ɍ쐬�ł��܂���B");
		}
	}
	for (i = 1; i < getCount(RevFiles); i++)
	for (j = 0; j < i; j++)
	{
		errorCase_m(!mbs_stricmp(getLine(RevFiles, i), getLine(RevFiles, j)), "�t�@�C�����X�g�ɏd��������܂��B");
	}
	for (i = 1; i < getCount(RevFiles); i++)
	{
		char *dir = changeLocal(getLine(RevFiles, i), "");

		if (*dir) // ? ���[�J��������Ȃ��B
		{
			for (j = 0; j < getCount(RevDirs); j++)
				if (!mbs_stricmp(dir, getLine(RevDirs, j)))
					break;

			errorCase_m(j == getCount(RevDirs), "�t�@�C�����X�g�̖��F�c���[�̔z���ɖ����t�@�C��������܂��B");
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
#else // old, �x��
	text = replaceLine(text, "\r\n", "\n", 0);
#endif

	lines = tokenize(text, '\n');
	memFree(text);

	errorCase_m(((char *)getLastElement(lines))[0], errorMessage); // ��̃t�@�C�����A���s�ŏI����Ă���͂��B

	memFree((char *)unaddElement(lines));

	foreach (lines, line, index)
	{
		errorCase_m(line[0] == '\0', errorMessage); // ��s�͖����͂��B
		errorCase_m(!isJLine(line, 1, 0, 0, 1), errorMessage);
	}
	return lines;
}
static void Check_IsCompactStamp(char *localDir)
{
	char *tmp = makeCompactStamp(getStampDataTime(compactStampToTime(localDir)));

	errorCase_m(strcmp(tmp, localDir), "���t�ɖ�肪����܂��B");

	memFree(tmp);
}
static void Check_CommentTxt(void)
{
	autoList_t *lines = CheckAndReadLines("comment.txt", "�R�����g�t�@�C�������Ă��܂��B");

	errorCase_m(getCount(lines) != 1, "�R�����g�t�@�C���̍s���ɖ�肪����܂��B");

	releaseDim(lines, 1);
}
static void Check_FilesTxt(void)
{
	autoList_t *lines = CheckAndReadLines("files.txt", "�t�@�C�����X�g�����Ă��܂��B");
	char *line;
	uint index;

	foreach (lines, line, index)
	{
		char *errorMessage = xcout("�t�@�C�����X�g�� %u �s�ڂɖ�肪����܂��B", index + 1);
		char *file;

		errorCase_m(!lineExp("<32,09AFaf> <>", line), errorMessage);
		file = line + 33;
		errorCase_m(!NoStrict && !isFairRelPath(file, 6), errorMessage); // �������Ă����������邩���H

		addElement(RevFiles, (uint)strx(file));

		memFree(errorMessage);
	}
	releaseDim(lines, 1);
}
static void Check_TreeTxt(void)
{
	autoList_t *lines = CheckAndReadLines("tree.txt", "�c���[�t�@�C�������Ă��܂��B");
	char *line;
	uint index;

	foreach (lines, line, index)
	{
		char *errorMessage = xcout("�c���[�t�@�C���� %u �s�ڂɖ�肪����܂��B", index + 1);

		errorCase_m(!NoStrict && !isFairRelPath(line, 6), errorMessage); // �������Ă����������邩���H

		addElement(RevDirs, (uint)strx(line));

		memFree(errorMessage);
	}
	releaseDim(lines, 1);
}

static void CheckRum(char *rumDir)
{
	errorCase_m(m_isEmpty(rumDir), "�w�肳�ꂽ�p�X�ɖ�肪����܂��B");
	errorCase_m(!existDir(rumDir), "�w�肳�ꂽ�f�B���N�g���͑��݂��܂���B");

	rumDir = makeFullPath(rumDir);

	errorCase_m(_stricmp(getExt(rumDir), "rum"), "�w�肳�ꂽ�p�X�̊g���q�ɖ�肪����܂��B");
	errorCase(!existDir(rumDir)); // 2bs

	addCwd(rumDir);

	// ---- .rum ���� ----

	errorCase_m(!existDir("files"), "files �f�B���N�g��������܂���B");
	errorCase_m(!existDir("revisions"), "revisions �f�B���N�g��������܂���B");

	{
		autoList_t *paths = ls(rumDir);

		errorCase_m(getCount(paths) != 2, "�]�v�ȃt�@�C���܂��̓f�B���N�g��������܂��B");

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

			errorCase_m(!existFile(file), "�t�@�C����������܂���B");

			localFile = getLocal(file);
			md5 = md5_makeHexHashFile(file);

			cout("%s\n", localFile);
			cout("%s\n", md5);

			errorCase_m(!lineExp("<32,09AFaf>", localFile), "�t�@�C�������t�H�[�}�b�g�Ɉ�v���܂���B");
			errorCase_m(_stricmp(localFile, md5), "�t�@�C�����j�����Ă��܂��B");
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

			errorCase_m(!existDir(dir), "�f�B���N�g����������܂���B");

			localDir = getLocal(dir);

			cout("%s\n", localDir);

			errorCase_m(!lineExp("<14,09>", localDir), "�f�B���N�g�������t�H�[�}�b�g�Ɉ�v���܂���B");
			Check_IsCompactStamp(localDir);

			// ---- revision ----

			addCwd(dir);

			errorCase_m(!existFile("comment.txt"), "�R�����g�t�@�C����������܂���B");
			errorCase_m(!existFile("files.txt"), "�t�@�C�����X�g��������܂���B");
			errorCase_m(!existFile("tree.txt"), "�c���[�t�@�C����������܂���B");

			{
				autoList_t *paths = ls(".");

				errorCase_m(getCount(paths) != 3, "�]�v�ȃt�@�C���܂��̓f�B���N�g��������܂��B");

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
	cout("| ���͌�����܂���ł��� |\n");
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
