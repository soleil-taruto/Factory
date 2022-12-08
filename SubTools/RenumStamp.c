/*
	RenumStamp.exe [/N] [/C] [/X] [�Ώ�DIR]

		/N ... �^�C���X�^���v�����̃t�@�C�����ɂ���B
		/C ... �쐬�������g���B
		/X ... �쐬�����E�X�V�����̂ǂ��炩�V���������g���B

		�f�t�H���g�ł͍X�V�������g���B
*/

#include "C:\Factory\Common\all.h"

static int BatchMode;
static int ToStampOnly;
static int TimeKind = 'W'; // "CWX" == create, write, �ǂ��炩�V������

static char *FStampsDir;
static char *HS_NamePtr;

static int HasStamp(char *file)
{
	if (lineExp("<4,09>-<2,09>-<2,09>_<2,09>-<2,09>-<2,09>_<3,09><>", file))
	{
		HS_NamePtr = file + 23;
		return 1;
	}
	return 0;
}
static char *GetFStamp(char *file) // ret: c_
{
	static char *ret;
	uint64 cTime;
	uint64 wTime;
	uint64 xTime;
	uint64 t;

	memFree(ret);

	getFileStamp(file, &cTime, NULL, &wTime);

	xTime = m_max(cTime, wTime);

	switch (TimeKind)
	{
	case 'C': t = cTime; break;
	case 'W': t = wTime; break;
	case 'X': t = xTime; break;

	default:
		error();
	}
	ret = xcout("%017I64u", t);

	// �d���̉��
	{
		addCwd(FStampsDir);
		ret = toCreatablePath(ret, IMAX);
		createFile(ret);
		unaddCwd();
	}

	errorCase(!lineExp("<17,09>", ret)); // 2bs?

	ret = insertChar(ret, 14, '_');
	ret = insertChar(ret, 12, '-');
	ret = insertChar(ret, 10, '-');
	ret = insertChar(ret, 8, '_');
	ret = insertChar(ret, 6, '-');
	ret = insertChar(ret, 4, '-');

	return ret;
}
static void DoFRenumStmp(void)
{
	autoList_t *files = lsFiles(".");
	autoList_t *midFiles = newList();
	autoList_t *destFiles = newList();
	char *file;
	uint index;
	uint fileCount;
	char *unqptn = xcout("_$u=%08I64x_", time(NULL)); // HACK: ���m���Ƀ��j�[�N�ɂȂ�悤��

	eraseParents(files);
	sortJLinesICase(files);

	FStampsDir = makeTempDir(NULL);

	foreach (files, file, index)
	{
		char *dest;

		errorCase(!*file); // 2bs

		if (ToStampOnly)
			dest = xcout("%s%s", GetFStamp(file), getExtWithDot(file));
		else if (HasStamp(file))
			dest = xcout("%s%s", GetFStamp(file), HS_NamePtr);
		else
			dest = xcout("%s_%s", GetFStamp(file), file);

		if (strcmp(file, dest)) // ? file != dest
		{
			cout("< %s\n", file);
			cout("> %s\n", dest);

			addElement(midFiles, (uint)xcout("%s%s", unqptn, dest));
			addElement(destFiles, (uint)dest);
		}
		else
		{
			file[0] = '\0';
		}
	}
	trimLines(files);
	errorCase(getCount(files) != getCount(midFiles)); // 2bs
	errorCase(getCount(files) != getCount(destFiles)); // 2bs

	recurRemoveDir(FStampsDir);
	memFree(FStampsDir);
	FStampsDir = NULL;

	if (!BatchMode)
	{
		cout("Press R to renumber_stamp\n");

		if (getKey() != 'R')
			termination(0);
	}

	LOGPOS();

	for (index = 0; index < getCount(files); index++) // �ړ��e�X�g�@
	{
		char *rFile = getLine(files, index);
		char *wFile = getLine(midFiles, index);

		errorCase(!existFile(rFile));
		errorCase( existFile(wFile));

		moveFile(rFile, wFile); // �ړ��g���C

		errorCase( existFile(rFile));
		errorCase(!existFile(wFile));

		moveFile(wFile, rFile); // ����

		errorCase(!existFile(rFile));
		errorCase( existFile(wFile));
	}

	LOGPOS();

	for (index = 0; index < getCount(files); index++)
		moveFile(
			getLine(files, index),
			getLine(midFiles, index)
			);

	LOGPOS();

	// �ړ��e�X�g�A
	{
		int errorFlag = 0;

		for (index = 0; index < getCount(files); index++)
		{
			char *wFile = getLine(destFiles, index);

			if (existPath(wFile))
			{
				cout("%s <���ɑ��݂���>\n", wFile);
				errorFlag = 1;
				break;
			}
			if (!creatable(wFile))
			{
				cout("%s <�쐬�s��>\n", wFile);
				errorFlag = 1;
				break;
			}
		}

		if (errorFlag)
		{
			for (index = 0; index < getCount(files); index++) // ����
				moveFile(
					getLine(midFiles, index),
					getLine(files, index)
					);

			error();
		}
	}

	LOGPOS();

	for (index = 0; index < getCount(files); index++)
		moveFile(
			getLine(midFiles, index),
			getLine(destFiles, index)
			);

	cout("Renumbered_Stamp!\n");

	releaseDim(files, 1);
	releaseDim(midFiles, 1);
	releaseDim(destFiles, 1);
	memFree(unqptn);
}
int main(int argc, char **argv)
{
readArgs:
	if (argIs("/B"))
	{
		BatchMode = 1;
		goto readArgs;
	}
	if (argIs("/N"))
	{
		ToStampOnly = 1;
		goto readArgs;
	}
	if (argIs("/C"))
	{
		TimeKind = 'C';
		goto readArgs;
	}
	if (argIs("/X"))
	{
		TimeKind = 'X';
		goto readArgs;
	}

	addCwd(hasArgs(1) ? nextArg() : c_dropDir());
	DoFRenumStmp();
	unaddCwd();
}
