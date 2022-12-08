/*
	MergeSrc.exe �}�X�^�[DIR �X���[�uDIR

	----

	�ȉ����X���[�u����}�X�^�[�ɏ㏑���R�s�[����B

		1. �}�X�^�[�ƃX���[�u�œ��e���قȂ�t�@�C��
		2. �X���[�u�ɂ��������t�@�C��
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\SubTools\libs\crlf.h"

#define CRLF_CONV_SIZE_MAX 165000

static char *RDir;
static char *WDir;
static autoList_t *RWFiles;

// ---- DoSearch ----

static int DS_IsSameFile(char *file1, char *file2)
{
	int ret;

	if (
		getFileSize(file1) < CRLF_CONV_SIZE_MAX &&
		getFileSize(file2) < CRLF_CONV_SIZE_MAX
		)
	{
		char *tmpFile1 = makeTempPath(NULL);
		char *tmpFile2 = makeTempPath(NULL);

		CRLF_ConvFile(file1, tmpFile1, "\n");
		CRLF_ConvFile(file2, tmpFile2, "\n");

		ret = isSameFile(tmpFile1, tmpFile2);

		removeFile_x(tmpFile1);
		removeFile_x(tmpFile2);
	}
	else
	{
		ret = isSameFile(file1, file2);
	}
	return ret;
}
static void DoSearch(void)
{
	autoList_t *files = lssFiles(RDir);
	char *rFile;
	uint index;

	sortJLinesICase(files);

	foreach (files, rFile, index)
	{
		char *wFile = changeRoot(strx(rFile), RDir, WDir);

		if (!existFile(wFile) || !DS_IsSameFile(rFile, wFile)) // ? �}�[�W���ׂ��t�@�C��
		{
			addElement(RWFiles, (uint)strx(eraseRoot(rFile, RDir)));
		}
		memFree(wFile);
	}
	releaseDim(files, 1);
}

// ---- DoConfirm ----

static double DC_FilePairingRate(char *iDir, char *oDir)
{
	autoList_t *files = lssFiles(iDir);
	char *iFile;
	uint index;
	uint n = 0;
	uint d;

	d = getCount(files);

	foreach (files, iFile, index)
	{
		char *oFile = changeRoot(strx(iFile), iDir, oDir);

		if (existFile(oFile))
			n++;

		memFree(oFile);
	}
	releaseDim(files, 1);

	return n * 1.0 / d;
}
static void DC_DispRWFiles(void)
{
	char *file;
	uint index;

	foreach (RWFiles, file, index)
	{
		cout("[%u] %s\n", index + 1, file);

		{
			char *file1 = combine(RDir, file);
			char *file2 = combine(WDir, file);

			if (
				existFile(file2) &&
				getFileSize(file1) < CRLF_CONV_SIZE_MAX &&
				getFileSize(file2) < CRLF_CONV_SIZE_MAX
				)
			{
				uint top1 = CRLF_GetTopFile(file1);
				uint top2 = CRLF_GetTopFile(file2);

				if (top1 != top2)
					cout("### ���s�R�[�h���Ⴂ�܂��B### %u %u\n", top1, top2);
			}
			memFree(file1);
			memFree(file2);
		}
	}
}
static void DC_OutputDiff(int keepTree)
{
	char *dir0;
	char *dir1;
	char *dir2;
	char *file;
	uint index;

	dir0 = makeFreeDir();
	dir1 = combine(dir0, "1_r_s");
	dir2 = combine(dir0, "2_w_m");

	createDir(dir1);
	createDir(dir2);

	foreach (RWFiles, file, index)
	{
		char *rFile = combine(RDir, file);
		char *wFile = combine(WDir, file);
		char *file1;
		char *file2;

		if (keepTree)
		{
			file1 = combine(dir1, file);
			file2 = combine(dir2, file);

			createPath(file1, 'X');
			createPath(file2, 'X');
		}
		else
		{
			char *file0 = strx(file);

			escapeYen(file0);
			file0 = replaceLine(file0, "/", "��", 0);

			file1 = combine(dir1, file0);
			file2 = combine(dir2, file0);

			memFree(file0);

			/*
				�d�������邯�ǃ��A�P�[�X�Ȃ̂ŁA����ł�����A�A�A
			*/
			file1 = toCreatablePath(file1, 1000);
			file2 = toCreatablePath(file2, 1000);
		}
		copyFile(rFile, file1);

		if (existFile(wFile))
			copyFile(wFile, file2);

		memFree(rFile);
		memFree(wFile);
		memFree(file1);
		memFree(file2);
	}
	execute_x(xcout("START \"\" \"%s\"", dir0));

	memFree(dir0);
	memFree(dir1);
	memFree(dir2);
}
static void EraseRWFiles(char *target)
{
	char *file;
	uint index;

	foreach (RWFiles, file, index)
		if (!_stricmp(file, target))
			file[0] = '\0';

	trimLines(RWFiles);
}
static void DoConfirm(void)
{
restart:
	execute("CLS");

	cout("%.3f %.3f\n", DC_FilePairingRate(RDir, WDir), DC_FilePairingRate(WDir, RDir));
	cout("< %s\n", RDir);
	cout("> %s\n", WDir);

	DC_DispRWFiles();

	cout("------------\n");
	cout("ENTER = ���s\n");
	cout("0 = �m�F�p�����o��_�K�w�i�V\n");
	cout("1 = �m�F�p�����o��_�K�w�A��\n");
	cout(". = .git* ���폜\n");
	cout("SPACE = �i�荞��\n");
	cout("OTHER = ���~\n");
	cout("------------\n");

	switch (getKey())
	{
	case 0x0d:
		break;

	case '0':
		DC_OutputDiff(0);
		goto restart;

	case '1':
		DC_OutputDiff(1);
		goto restart;

	case '.':
		EraseRWFiles(".gitattributes");
		EraseRWFiles(".gitignore");
		goto restart;

	case 0x20:
		RWFiles = selectLines_x(RWFiles);
		goto restart;

	default:
		termination(0);
	}

	{
		const char *CFM_PTN = "WRITE";
		char *line;

		cout("### �m�F�̂��� %s �Ɠ��͂��ĂˁB### (ignore case)\n", CFM_PTN);
		line = coInputLine();

		if (_stricmp(line, CFM_PTN)) // ? ��v���Ȃ��B
		{
			memFree(line);
			goto restart;
		}
		memFree(line);
	}
}

// ---- DoMerge ----

static void DoMerge(void)
{
	char *file;
	uint index;

	foreach (RWFiles, file, index)
	{
		char *rFile = combine(RDir, file);
		char *wFile = combine(WDir, file);
		char *retCode = NULL;

		cout("< %s\n", rFile);
		cout("> %s\n", wFile);

		if (existFile(wFile))
			semiRemovePath(wFile);
		else
			createPath(wFile, 'X');

		copyFile(rFile, wFile);
	}
}

// ----

static void MergeDir(char *masterDir, char *slaveDir)
{
	errorCase(m_isEmpty(masterDir));
	errorCase(m_isEmpty(slaveDir));

	masterDir = makeFullPath(masterDir);
	slaveDir = makeFullPath(slaveDir);

	errorCase(!existDir(masterDir));
	errorCase(!existDir(slaveDir));
	errorCase(!mbs_stricmp(masterDir, slaveDir)); // �}�X�^�[�ƃX���[�u������DIR

	cout("��������������������������������\n");
	cout("���\�[�X�t�@�C�����}�[�W����恡\n");
	cout("��������������������������������\n");
	cout("[�}�X�^�[] > %s\n", masterDir);
	cout("[�X���[�u] < %s\n", slaveDir);
	cout("���s�H\n");

	if (getKey() == 0x1b)
		termination(0);

	cout("���s���܂��B\n");

	RDir = slaveDir;
	WDir = masterDir;
	RWFiles = newList();

	DoSearch();
	DoConfirm();
	DoMerge();
}
int main(int argc, char **argv)
{
	MergeDir(getArg(0), getArg(1));
}
