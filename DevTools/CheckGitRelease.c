/*
	CheckGitRelease [/D ���ڃ`�F�b�NDIR | ���[�gDIR]
*/

#include "C:\Factory\Common\all.h"

#define LOCAL_GIT_RELEASE_BAT "GitRelease.bat"
#define LOCAL_RUN_ME_FIRST_BAT "__run_me_first.bat_"
#define LOCAL_LICENSE "LICENSE"
#define LOCAL_LICENSE_IGNORE "LIGNORE" // LICENSE�t�@�C�����܂��H�ݒ肵�Ȃ��ꍇ�A�b��I�ɁH�����u���Ă����B

static autoList_t *KnownRepos;
static autoList_t *ErrorFiles;
static int ErrorFound;

static void FoundError(char *message)
{
	cout("�� %s\n", message);
	ErrorFound = 1;
}
static char *ProjectLocalDirToRepositoryName(char *localDir)
{
	if (lineExp("<1,azAZ><8,09>_<1,,>", localDir))
	{
		localDir += 10;
	}
	return localDir;
}
static void CheckGitRelease(char *dir)
{
	dir = makeFullPath(dir);
	cout("�`�F�b�N�Ώۃf�B���N�g�� �� %s\n", dir);
	ErrorFound = 0;

	addCwd(dir);

	if (!existFile(LOCAL_GIT_RELEASE_BAT))
	{
		FoundError(LOCAL_GIT_RELEASE_BAT " ��������܂���B");
	}
	else
	{
		{
			char *text = readText_b(LOCAL_GIT_RELEASE_BAT);
			char *text2 = xcout(
				"@rem ���̃t�H���_�̃��[�J������ Repo �܂��� x99999999_Repo (Repo=�����[�X��̃��|�W�g����) �ł��邱�ƁB\r\n"
				"\r\n"
				"IF NOT EXIST .\\GitRelease.bat GOTO END\r\n"
				"CALL qq\r\n"
				"C:\\Factory\\SubTools\\GitFactory.exe /ow . C:\\huge\\GitHub\\%s\r\n"
				"rem CALL C:\\temp\\go.bat\r\n"
				":END\r\n"
				,ProjectLocalDirToRepositoryName(getLocal(dir))
				);

			if (strcmp(text, text2))
			{
				FoundError(LOCAL_GIT_RELEASE_BAT " �̓��e�ɖ�肪����܂��B(bad contents)");
			}
			memFree(text);
			memFree(text2);
		}

		{
			char *repoDir = combine("C:\\huge\\GitHub", ProjectLocalDirToRepositoryName(getLocal(dir)));

			if (!existDir(repoDir))
			{
				FoundError(LOCAL_GIT_RELEASE_BAT " �̓��e�ɖ�肪����܂��B(no repoDir)");
			}
			memFree(repoDir);
		}

		{
			char *repo = ProjectLocalDirToRepositoryName(getLocal(dir));

			if (findJLineICase(KnownRepos, repo) < getCount(KnownRepos))
			{
				FoundError(LOCAL_GIT_RELEASE_BAT " �̓��e�ɖ�肪����܂��B(���|�W�g���̏d��)");
			}
			else
			{
				addElement(KnownRepos, (uint)strx(repo));
			}
		}
	}

	if (!existFile(LOCAL_RUN_ME_FIRST_BAT))
	{
		FoundError(LOCAL_RUN_ME_FIRST_BAT " ��������܂���B");
	}
	else
	{
		char *text = readText_b(LOCAL_RUN_ME_FIRST_BAT);
		char *text2 = strx(
			"C:\\Factory\\bat\\devrunmefirst.bat\r\n"
			);

		if (strcmp(text, text2))
		{
			FoundError(LOCAL_RUN_ME_FIRST_BAT " �̓��e�ɖ�肪����܂��B");
		}
		memFree(text);
		memFree(text2);
	}

	if (m_01(existFile(LOCAL_LICENSE)) + m_01(existFile(LOCAL_LICENSE_IGNORE)) != 1)
	{
		FoundError("���C�Z���X�t�@�C�� (LICENSE) ��������܂���B");
	}
	if (existFile(LOCAL_LICENSE) && getFileSize(LOCAL_LICENSE) == 0)
	{
		FoundError("���C�Z���X�t�@�C�� (LICENSE) ������ۂł��B");
	}
	if (existFile(LOCAL_LICENSE_IGNORE) && getFileSize(LOCAL_LICENSE_IGNORE) != 0)
	{
		FoundError(LOCAL_LICENSE_IGNORE " ����ł͂���܂���B");
	}

	unaddCwd();

	if (ErrorFound)
		addElement(ErrorFiles, (uint)combine(dir, LOCAL_GIT_RELEASE_BAT));

	memFree(dir);
}

static void CheckDir(char *dir);

static void FindProjectDir(char *rootDir)
{
	autoList_t *dirs = lsDirs(rootDir);
	char *dir;
	uint index;

	foreach (dirs, dir, index)
	{
		CheckDir(dir);
	}
	releaseDim(dirs, 1);
}
static void CheckDir(char *dir)
{
	char *gitReleaseBat = combine(dir, LOCAL_GIT_RELEASE_BAT);

	if (existFile(gitReleaseBat))
	{
		CheckGitRelease(dir);
	}
	else
	{
		FindProjectDir(dir);
	}
	memFree(gitReleaseBat);
}
int main(int argc, char **argv)
{
	KnownRepos = newList();
	ErrorFiles = newList();

	errorCase_m(argIs("/C"), "�p�~�I�v�V����"); // zantei

	if (argIs("/D"))
	{
		CheckGitRelease(nextArg());
	}
	else if (hasArgs(1))
	{
		CheckDir(nextArg());
	}
	else
	{
		CheckDir("C:\\Dev");
	}

	cout("\n");

	if (getCount(ErrorFiles))
	{
		cout("������ [ %u ] ���̃G���[��������܂����B\n", getCount(ErrorFiles));
	}
	else
	{
		cout("�G���[�͌�����܂���ł����B\n");
	}
	writeLines(FOUNDLISTFILE, ErrorFiles);
}
