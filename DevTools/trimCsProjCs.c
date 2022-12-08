/*
	trimCsProjCs.exe [/S] [/D | �\�����[�V����������f�B���N�g��]
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\xml.h"

#define Clean() \
	(coExecute("solclean ."))
//	(coExecute("qq -f")) // 1.txt �Ƀ��_�C���N�g�����Ƃ��}�Y���B

#define Build() \
	(coExecute("cx *"))

#define WriteProjFile(file, lines) \
	(writeOneLineNoRet_cx((file), untokenize((lines), "\n")))

static char *SlnFile;
static char *ProjFile;
static autoList_t *ProjLines;
static char *AsmbName;
static char *ProjDir;
static char *OutFile;
static char *ProjBackupFile;
static autoList_t *DeletableCsFiles;

static int TryBuild(void)
{
	WriteProjFile(ProjFile, ProjLines);

	Clean();
	Build();

	removeFile(ProjFile);

	return existFile(OutFile);
}
static int TrimProjLines(void)
{
	char *line;
	uint index;
	int trimmed = 0;

	foreach (ProjLines, line, index)
	{
		if (startsWith(line, "    <Compile Include=\"") && endsWith(line, "\" />"))
		{
			char *csFile = ne_strchr(line, '"') + 1;
			char *p;

			p = ne_strchr(csFile, '"');
			*p = '\0';
			csFile = strx(csFile);
			*p = '"';

			if (
				isFairRelPath(csFile, strlen(ProjDir)) && // ? SJIS�t�@�C���� && ProjDir�̔z��(".."���܂܂Ȃ�����΃p�X�ł͂Ȃ�)
				!_stricmp(getExt(csFile), "cs") &&        // ? .cs�t�@�C��
				!startsWithICase(csFile, "Properties\\") && // ? ! ���O
				_stricmp(getLocal(csFile), "Program.cs")    // ? ! ���O
				)
			{
				cout("csFile.1: %s\n", csFile);

				csFile = combine_cx(ProjDir, csFile);

				cout("csFile.2: %s\n", csFile);

				if (existFile(csFile))
				{
					LOGPOS();
					desertElement(ProjLines, index);

					if (TryBuild())
					{
						LOGPOS();
						memFree(line);
						addElement(DeletableCsFiles, (uint)strx(csFile));
						trimmed = 1;
					}
					else
					{
						insertElement(ProjLines, index, (uint)line); // ���ɖ߂��B
					}
					LOGPOS();
				}
			}
			memFree(csFile);
		}
	}

	Clean();

	cout("trimmed: %d\n", trimmed);
	return trimmed;
}
static int ConfirmDeleteCsFiles(void)
{
	char *file;
	uint index;
	int ret;

	cout("\n----\n");

	foreach (DeletableCsFiles, file, index)
		cout("�폜�ΏہF%s\n", file);

	cout("�폜����ɂ� 'D' ����͂��ĂˁB\n");
	ret = clearGetKey() == 'D';
	cout("ret: %d\n", ret);
	sleep(500); // cout��������悤��
	return ret;
}
static void ProcProj(int checkOnly)
{
	cout("checkOnly: %d\n", checkOnly);
	cout("ProjFile: %s\n", ProjFile);

	ProjLines = readLines(ProjFile);

	// check ProjLines �������o���� ProjFile ���Č��ł��邩�ǂ��� -- 2bs
	{
		char *wkFile = makeTempPath(NULL);

		WriteProjFile(wkFile, ProjLines);

		errorCase(!isSameFile(wkFile, ProjFile));

		removeFile(wkFile);
		memFree(wkFile);
	}

	// set AsmbName
	{
#if 1
		XNode_t *xml = readXMLFile(ProjFile);

		AsmbName = strx(ne_GetXNode(xml, "PropertyGroup/AssemblyName")->Text);

		ReleaseXNode(xml);
#else // old
		char *line;
		uint index;
		char *p;

		foreach (ProjLines, line, index)
			if (startsWith(line, "    <AssemblyName>") && endsWith(line, "</AssemblyName>"))
				break;

		errorCase(!line);

		AsmbName = ne_strchr(line, '>') + 1;

		p = ne_strchr(AsmbName, '<');
		*p = '\0';
		AsmbName = strx(AsmbName);
		*p = '<';
#endif

		errorCase_m(!isJToken(AsmbName, 1, 0), "�A�Z���u�����ɖ�肪����܂��B");
	}

	cout("AsmbName: %s\n", AsmbName);

	ProjDir = getParent(ProjFile);

	cout("ProjDir: %s\n", ProjDir);

	Clean();
	Build();

	{
		autoList_t *files = lssFiles(ProjDir);
		char *file;
		uint index;

		foreach (files, file, index)
		{
			if (
				!mbs_stristr(file, "\\bin\\Release\\") ||
				!mbs_stristr(getLocal(file), AsmbName) ||
				_stricmp(getExt(file), "exe") && _stricmp(getExt(file), "dll")
				)
				file[0] = '\0';
		}
		trimLines(files);

		errorCase_m(getCount(files) != 1, "�o�̓t�@�C�����P�ɍi��܂���B");

		OutFile = getLine(files, 0);
		releaseAutoList(files);
	}

	cout("OutFile: %s\n", OutFile);

	Clean();

	errorCase_m(existFile(OutFile), "�N���[���A�b�v���Ă��o�̓t�@�C�����폜����܂���B");

	ProjBackupFile = addExt(strx(ProjFile), "bak");

	cout("ProjBackupFile: %s\n", ProjBackupFile);

	errorCase_m(existPath(ProjBackupFile), "�o�b�N�A�b�v�t�@�C�����c���Ă��܂��B");

	if (checkOnly)
		goto checkOnlyEnd;

	LOGPOS();

	moveFile(ProjFile, ProjBackupFile);

	DeletableCsFiles = newList();

	while (TrimProjLines());

	if (ConfirmDeleteCsFiles())
	{
		char *file;
		uint index;

		LOGPOS();
		WriteProjFile(ProjFile, ProjLines);

		foreach (DeletableCsFiles, file, index)
			semiRemovePath(file);

		semiRemovePath(ProjBackupFile);
	}
	else
	{
		moveFile(ProjBackupFile, ProjFile);
	}

	LOGPOS();
	releaseDim(DeletableCsFiles, 1);
	DeletableCsFiles = NULL;

	LOGPOS();

checkOnlyEnd:
	releaseDim(ProjLines, 1);
	memFree(AsmbName);
	memFree(ProjDir);
	memFree(OutFile);
	memFree(ProjBackupFile);
	ProjLines = NULL;
	AsmbName = NULL;
	ProjDir = NULL;
	OutFile = NULL;
	ProjBackupFile = NULL;

	LOGPOS();
}
static void TrimCsProjCs(void)
{
	// set SlnFile
	{
		autoList_t *files = lsFiles("."); // �J�����g�̂ݒT���B
		char *file;
		uint index;

		foreach (files, file, index)
			if (_stricmp(getExt(file), "sln"))
				file[0] = '\0';

		trimLines(files);

		errorCase_m(getCount(files) != 1, "�\�����[�V�������P�ɍi��܂���B�\�����[�V�����t�@�C���Ɠ����ꏊ������s���ĉ������B");

		SlnFile = getLine(files, 0);
		releaseAutoList(files);
	}

	cout("SlnFile: %s\n", SlnFile);

	// check SlnFile -- vcs 2010 express ���ǂ���
	{
		FILE *fp = fileOpen(SlnFile, "rt");

		c_neReadLine(fp);
		errorCase_m(strcmp(c_neReadLine(fp), "Microsoft Visual Studio Solution File, Format Version 11.00"), "vs2010����Ȃ��ł���B");
		errorCase_m(strcmp(c_neReadLine(fp), "# Visual C# Express 2010"), "C#��Express����Ȃ��ł���B");

		fileClose(fp);
	}

	// set ProjFile + call (CheckProj, ProcProj)
	{
		autoList_t *files = lssFiles(".");
		char *file;
		uint index;

		foreach (files, file, index)
			if (_stricmp(getExt(file), "csproj"))
				file[0] = '\0';

		trimLines(files);

		errorCase_m(!getCount(files), "�v���W�F�N�g���P���L��܂���B");

		foreach (files, file, index)
		{
			ProjFile = file;
			ProcProj(1);
			ProjFile = NULL;
		}
		foreach (files, file, index)
		{
			ProjFile = file;
			ProcProj(0);
			ProjFile = NULL;
		}
		releaseDim(files, 1);
	}

	memFree(SlnFile);
	SlnFile = NULL;
}

static int SearchSubDirFlag;

static void Main2(void)
{
	if (SearchSubDirFlag)
	{
		autoList_t *files = lssFiles(".");
		char *file;
		uint index;

		foreach (files, file, index)
			if (_stricmp(getExt(file), "sln"))
				file[0] = '\0';

		trimLines(files);

		files = selectLines_x(files);

		foreach (files, file, index)
		{
			char *dir = getParent(file);

			addCwd(dir);
			{
				TrimCsProjCs();
			}
			unaddCwd();
		}
		releaseDim(files, 1);
	}
	else
	{
		TrimCsProjCs();
	}
}
int main(int argc, char **argv)
{
	if (argIs("/S"))
	{
		SearchSubDirFlag = 1;
	}

	if (argIs("/D"))
	{
		char *slnDir = c_dropDir();

		addCwd(slnDir);
		{
			Main2();
		}
		unaddCwd();
	}
	else if (hasArgs(1))
	{
		char *slnDir = nextArg();

		addCwd(slnDir);
		{
			Main2();
		}
		unaddCwd();
	}
	else
	{
		Main2();
	}
}
