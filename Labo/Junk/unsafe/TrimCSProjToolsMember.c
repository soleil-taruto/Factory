/*
	Tools\ ������ .cs ���̎g�p���ĂȂ������o (���\�b�h�E�t�B�[���h�E�v���p�e�B) ���폜����B

	----

	.sln ������ꏊ������s���ĂˁB(C#)

	�v���W�F�N�g������ Tools\ ������ .cs �����������܂��B

	.cs ���̃����o (���\�b�h�E�t�B�[���h�E�v���p�e�B) ���폜���܂��B
	.cs �͍폜���܂���B

	.exe �̃v���W�F�N�g����Ȃ���΂Ȃ�Ȃ��B<---- Tools ������Ȃ瑽�� .exe �ł��傤�B
*/

#include "C:\Factory\Common\all.h"

#define DIR_TOOLS "Tools"

static char *ProjName;
static char *BuiltExeFile;

static int TryBuild_NoCD(void)
{
	coExecute("qq");

	errorCase(existFile(BuiltExeFile));

	coExecute("cx **");

	return existFile(BuiltExeFile);
}
static int TryBuild(void)
{
	int ret;

	addCwd("..\\..");
	{
		ret = TryBuild_NoCD();
	}
	unaddCwd();

	return ret;
}

static uint IM_StartLineIndex;
static uint IM_EndLineIndex;

/*
	�N���X�����o�̐錾���ł��邩�`�F�b�N����B

	�N���X�����o�̐錾���ł������ꍇ�A�ȉ���ݒ肷��B
		IM_StartLineIndex  <--  ���Y�����o�̊J�n�s�C���f�b�N�X
		IM_EndLineIndex    <--  ���Y�����o�̍ŏI�s�C���f�b�N�X + 1

	lines: �\�[�X
	targLineIndex: �`�F�b�N����s�C���f�b�N�X
	ret: ? �N���X�����o�̐錾���ł���B
*/
static int IsMember(autoList_t *lines, uint targLineIndex)
{
	char *line = getLine(lines, targLineIndex);
	uint index;
	int status;

	status =
		startsWith(line, "\t\tpublic ") ||
		startsWith(line, "\t\tprivate ") ||
		startsWith(line, "\t\tprotected ");

	if (!status)
		return 0;

	// ��[������ ...

	for (index = targLineIndex; index; index--)
	{
		line = getLine(lines, index - 1);
		status =
			startsWith(line, "\t\t[") ||
			startsWith(line, "\t\t/// ");

		if (!status)
			break;
	}
	IM_StartLineIndex = index;

	// ���[������ ...

	index = targLineIndex;
	index++;
	line = refLine(lines, index);

	if (startsWith(line, "\t\t\t: "))
	{
		index++;
		line = refLine(lines, index);

		if (strcmp(line, "\t\t{ }"))
			return 0;

		IM_EndLineIndex = index + 1;
		return 1;
	}
	if (!strcmp(line, "\t\t{ }"))
	{
		IM_EndLineIndex = index + 1;
		return 1;
	}
	if (strcmp(line, "\t\t{"))
	{
		IM_EndLineIndex = index;
		return 1;
	}
	for (; ; )
	{
		index++;

		if (getCount(lines) <= index)
			return 0;

		line = getLine(lines, index);

		if (!strcmp(line, "\t\t}"))
			break;

		status =
			!*line ||
			startsWith(line, "#") ||
			startsWith(line, "\t\t\t");

		if (!status)
			return 0;
	}
	IM_EndLineIndex = index + 1;
	return 1;
}
static void EraseMember(autoList_t *lines, uint startLineIndex, uint endLineIndex)
{
	uint index;

	for (index = startLineIndex; index < endLineIndex; index++)
		*getLine(lines, index) = '\0';
}
static void Main2_Tools(void)
{
	autoList_t *files = lsFiles(".");
	char *file;
	uint index;
	autoList_t *deletableCsFiles = newList();
	char *escapedFile = makeTempPath(NULL);
	int deletedFlag;

LOGPOS();
restart:
LOGPOS();
	errorCase(!TryBuild());

	deletedFlag = 0;

LOGPOS();
	foreach (files, file, index)
	{
		if (!_stricmp("cs", getExt(file)))
		{
			autoList_t *lines = readLines(file); // �\�[�X�t�@�C������ǂݍ���
			char *line;
			uint line_index;

			foreach (lines, line, line_index)
			{
				if (IsMember(lines, line_index))
				{
					autoList_t *emLines = copyLines(lines);
					uint startLineIndex = IM_StartLineIndex;
					uint endLineIndex   = IM_EndLineIndex;

					cout("TRY-ERASE-MEMBER.1\n");

					cout("TEM_file: %s\n", file);
					cout("TEM_stLn: %u\n", startLineIndex);
					cout("TEM_edLn: %u\n", endLineIndex);

					EraseMember(emLines, startLineIndex, endLineIndex);

					writeLines(file, emLines);
copyFile_cx(file, toCreatableTildaPath(combine("C:\\temp", getLocal(file)), IMAX)); // test test test test test

					if (TryBuild())
					{
						LOGPOS();
						releaseDim(lines, 1);
						lines = emLines;
						deletedFlag = 1;
					}
					else
					{
						releaseDim(emLines, 1);
					}

					cout("TRY-ERASE-MEMBER.2\n");
				}
			}
			writeLines(file, lines); // �\�[�X�t�@�C���𕜌�����B

			releaseDim(lines, 1);
		}
	}
LOGPOS();

	if (deletedFlag)
		goto restart;

LOGPOS();
	foreach (deletableCsFiles, file, index)
	{
		removeFile(file);
	}
LOGPOS();
	releaseDim(files, 1);
	releaseDim(deletableCsFiles, 1);
	memFree(escapedFile);
LOGPOS();
}
static void Main2_Project(void)
{
	char *csProjFile = xcout("%s.csproj", ProjName);

	errorCase(!existFile(csProjFile));
	errorCase(!existDir(DIR_TOOLS));

	addCwd(DIR_TOOLS);
	{
		Main2_Tools();
	}
	unaddCwd();
}
static void Main2_Solution(void)
{
	autoList_t *files = lsFiles(".");
	char *file;
	uint index;
	char *slnFile = NULL;
	char *projName;

	foreach (files, file, index)
	{
		if (!_stricmp("sln", getExt(file)))
		{
			errorCase(slnFile); // ? 2��
			slnFile = strx(file);
		}
	}
	errorCase(!slnFile); // ? 1�������B

	ProjName = changeExt(getLocal(slnFile), "");
	BuiltExeFile = combine_xx(getCwd(), xcout("%s\\bin\\Release\\%s.exe", ProjName, ProjName));

	errorCase(!existDir(ProjName));

	addCwd(ProjName);
	{
		Main2_Project();
	}
	unaddCwd();

	releaseDim(files, 1);
	memFree(slnFile);
}
int main(int argc, char **argv)
{
LOGPOS();
	Main2_Solution();
LOGPOS();
}
