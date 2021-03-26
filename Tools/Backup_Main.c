#include "C:\Factory\Common\all.h"
#include "C:\Factory\Meteor\Toolkit.h"

static int NoShutdownFlag;

static void ToDecAlphaOnly(char *line)
{
	char *p;

	for(p = line; *p; p++)
	{
		if(!m_isdecimal(*p) && !m_isalpha(*p))
		{
			*p = '_';
		}
	}
}
static void ToOnomast(char *line)
{
	char *p;

	line[0] = m_toupper(line[0]);

	for(p = line + 1; *p; p++)
	{
		*p = m_tolower(*p);
	}
}

static autoList_t *GetTargetDirs(void)
{
	autoList_t *paths = ls("C:\\");
	autoList_t dirs;
	char *dir;
	uint index;
	autoList_t *retDirs = createAutoList(16);
	char *userName;

	dirs = gndSubElements(paths, 0, lastDirCount);

	foreach(&dirs, dir, index)
	{
		if(
			!_stricmp(dir, "C:\\Documents and Settings") ||
			!_stricmp(dir, "C:\\PerfLogs") || // for 10
			!_stricmp(dir, "C:\\ProgramData") || // for 7
			!_stricmp(dir, "C:\\Program Files") ||
			!_stricmp(dir, "C:\\Program Files (x86)") || // for 7 x64
			!_stricmp(dir, "C:\\RECYCLER") ||
			!_stricmp(dir, "C:\\System Volume Information") ||
			!_stricmp(dir, "C:\\WINDOWS") || // for XP, 7
			!_stricmp(dir, "C:\\Windows.old") || // for 10
			!_stricmp(dir, "C:\\WINNT") || // for 2000
			!_stricmp(dir, "C:\\$Recycle.Bin") || // for 7
			!_stricmp(dir, "C:\\Config.Msi") || // for 7
			!_stricmp(dir, "C:\\Recovery") || // for 7
			!_stricmp(dir, "C:\\Users") || // for 7
			!_stricmp(dir, "C:\\$SysReset") || // for 10 �S�~
			!_stricmp(dir, "C:\\Windows.old") || // for 10 �S�~
			!_stricmp(dir, "C:\\MSOCache") || // for 7, 10 Office 2010

			dir[3] == '_' || m_isdecimal(dir[3]) || // _ 0�`9 �Ŏn�܂�t�H���_�͑ΏۊO

//			!_stricmp(dir, "C:\\huge") || // del @ 2017.12.16

			!_stricmp(dir, "C:\\$WinREAgent") || // add @ 2021.3.24

			0
			)
		{
			cout("�����O�F%s\n", dir);
		}
		else
		{
			addElement(retDirs, (uint)strx(dir));
		}
	}
	releaseDim(paths, 1);
	sortJLinesICase(retDirs);
//	rapidSortLines(retDirs); // old

	userName = getenv("UserName");

	errorCase(!userName);
	errorCase(!userName[0]);

	if(existDir("C:\\Users")) // ? 7
	{
		addElement(retDirs, (uint)xcout("C:\\Users\\%s\\Favorites", userName));
//		addElement(retDirs, (uint)xcout("C:\\Users\\%s\\Documents", userName)); // �����������Ƃ�������B
		addElement(retDirs, (uint)xcout("C:\\Users\\%s\\Desktop", userName));
	}
	else // ? XP
	{
		addElement(retDirs, (uint)xcout("C:\\Documents and Settings\\%s\\Favorites", userName));
		addElement(retDirs, (uint)xcout("C:\\Documents and Settings\\%s\\My Documents", userName));
		addElement(retDirs, (uint)xcout("C:\\Documents and Settings\\%s\\�f�X�N�g�b�v", userName));
	}
	return retDirs;
}
static void CheckTargetDirs(autoList_t *dirs)
{
	autoList_t *localDirs = createAutoList(getCount(dirs));
	char *dir;
	uint index;

	foreach(dirs, dir, index)
	{
		cout("%s\n", dir);
		errorCase(!existDir(dir));
		addElement(localDirs, (uint)getLocal(dir));
	}
	cout("\n");

	foreach(localDirs, dir, index)
	{
		autoList_t followDirs = gndFollowElements(localDirs, index + 1);
		errorCase(findJLineICase(&followDirs, dir) < getCount(&followDirs)); // ? �d�������B
	}
	releaseAutoList(localDirs);
}

#define BATCH_BACKUP "C:\\Factory\\tmp\\Backup.bat"

static int ELTD_IsTargetDir(char *destDir, autoList_t *targetDirs)
{
	char *targetDir;
	uint index;

	foreach(targetDirs, targetDir, index)
		if(!_stricmp(destDir, getLocal(targetDir)))
			return 1;

	return 0;
}
static void EraseLostTargetDirs(autoList_t *targetDirs)
{
	autoList_t *destDirs = lsDirs(".");
	char *destDir;
	uint index;

	eraseParents(destDirs);
	sortJLinesICase(destDirs);

	foreach(destDirs, destDir, index)
	{
		FILE *fp;
		char *line;

		cout("%s\n", destDir);

		if(ELTD_IsTargetDir(destDir, targetDirs))
		{
			cout("���̃t�H���_�̓^�[�Q�b�g�ł��B-> �폜���Ȃ��B\n");
			continue;
		}
		cout("���̃t�H���_�̓^�[�Q�b�g�ł͂���܂���B-> �폜����B\n");

		fp = fileOpen(BATCH_BACKUP, "wt");

//		writeLine(fp, "SET COPYCMD=");
		writeLine(fp, line = xcout("RD /S /Q \"%s\"", destDir)); memFree(line);
//		writeLine(fp, line = xcout("ROBOCOPY.EXE \"%s\" \"%s\" /MIR", targetDir, destDir)); memFree(line);
//		writeLine(fp, "ECHO ERRORLEVEL=%ERRORLEVEL%");

		fileClose(fp);

		cout("���X�g�����t�H���_���폜���Ă��܂�...\n");

		execute(line = xcout("START /B /WAIT CMD /C %s", BATCH_BACKUP)); memFree(line);

		cout("���X�g�����t�H���_���폜���܂����B\n");
		cout("\n");
	}
	releaseDim(destDirs, 1);
}
static void BackupDirs(autoList_t *targetDirs)
{
	char *targetDir;
	uint index;

	foreach(targetDirs, targetDir, index)
	{
		char *destDir = getLocal(targetDir);
		FILE *fp;
		char *line;

		cmdTitle_x(xcout("Backup - %u / %u (%u) -S=%d", index, getCount(targetDirs), getCount(targetDirs) - index, NoShutdownFlag));

		fp = fileOpen(BATCH_BACKUP, "wt");

		writeLine(fp, "SET COPYCMD=");
		writeLine(fp, line = xcout("MD \"%s\"", destDir)); memFree(line);
		writeLine(fp, line = xcout("ROBOCOPY.EXE \"%s\" \"%s\" /MIR", targetDir, destDir)); memFree(line);
		writeLine(fp, "ECHO ERRORLEVEL=%ERRORLEVEL%");

		fileClose(fp);

		cout("�R�s�[���Ă��܂�...\n");

		execute(line = xcout("START /B /WAIT CMD /C %s", BATCH_BACKUP)); memFree(line);

		cout("�R�s�[���܂����B\n");
		cout("\n");
	}
	cmdTitle("Backup - done");
}
int main(int argc, char **argv)
{
	char *strDestDrv;
	int destDrv;
	char *pcname;
	char *destDir;
	autoList_t *targetDirs;
	char *cmdln;
	int doShutdownFlag;

	// �O���R�}���h���݊m�F
	{
		errorCase(!existFile(FILE_TOOLKIT_EXE));
	}

	NoShutdownFlag = argIs("/-S");

	if(!NoShutdownFlag)
	{
		cout("********************************\n");
		cout("** �I����V���b�g�_�E�����܂� **\n");
		cout("********************************\n");
	}
	cout("+----------------------------------+\n");
	cout("| �o�b�N�A�b�v��� P �h���C�u�ł��B|\n");
	cout("+----------------------------------+\n");
	cout("+---------------------------------------------+\n");
	cout("| ���[���[�ȂǁA���쒆�̃A�v������ĉ������B|\n");
	cout("+---------------------------------------------+\n");
//	cout("+---------------------------------------+\n");
//	cout("| �E�B���X�΍�\�t�g�𖳌��ɂ��ĉ������B|\n");
//	cout("+---------------------------------------+\n");

	{
		cout("���s�H\n");

		if(clearGetKey() == 0x1b)
			termination(0);

		cout("���s���܂��B\n");
	}

	strDestDrv = strx("P:\\");
	errorCase_m(!existDir(strDestDrv), "�o�b�N�A�b�v��� P �h���C�u�����݂��܂���B");
	destDrv = strDestDrv[0];

//	errorCase(!m_isalpha(destDrv));
//	errorCase(destDrv == 'C');

	pcname = getenv("ComputerName");

	errorCase(!pcname);
	errorCase(!pcname[0]);
	errorCase(strchr(pcname, ' '));

	pcname = strx(pcname);
	ToDecAlphaOnly(pcname);
	ToOnomast(pcname);
	destDir = xcout("%c:\\%s", destDrv, pcname);
	memFree(pcname);

	cout("�o�͐�: %s\n", destDir);
	cout("\n");

	targetDirs = GetTargetDirs();
	CheckTargetDirs(targetDirs);

	createDirIfNotExist(destDir);
	coExecute_x(xcout("Compact.exe /C \"%s\"", destDir));
//	coExecute_x(xcout("Compact.exe /C /S:\"%s\"", destDir));

	addCwd(destDir);
	coExecute("DEL /Q *"); // destDir �̒����̃t�@�C����S�č폜����B
	EraseLostTargetDirs(targetDirs);
	BackupDirs(targetDirs);
	unaddCwd();

	// _ 0�`9 �Ŏn�܂閼�O�̓o�b�N�A�b�v�ΏۊO�Ȃ̂ŁAdestDir �̒����� _ 0�`9 �Ŏn�߂Ă����Ώd�����Ȃ��͂��B

#if 0 // �}�~ @ 2020.10.24
	// Toolkit �� USB HDD �ւ̏������݃G���[���N�������̂ŁA�������ݐ���V�X�e���h���C�u�ɂ��ėl�q������B@ 2018.3.5
	// �n�b�V�����X�g�쐬
	{
		char *midDir = makeTempDir("Backup_Hash_txt_mid");

		coExecute_x(xcout(FILE_TOOLKIT_EXE " /SHA-512-128 %s %s\\_Hash.txt", destDir, midDir));

		coExecute_x(xcout("COPY /Y %s\\_Hash.txt %s\\_Hash.txt", midDir, destDir));
		coExecute_x(xcout("COPY /Y %s\\_Hash.txt C:\\tmp\\Backup_Hash.txt", midDir));

		recurRemoveDir_x(midDir);
	}
#endif

	// Backup.bat �Ƃ̘A�g
	{
		if(existDir("C:\\888"))
		{
			writeOneLine_cx("C:\\888\\Extra_0001.bat", xcout("COPY /Y C:\\vaz\\Backup.log %s\\_Backup.log", destDir));
		}
	}

	memFree(strDestDrv);
	memFree(destDir);
	releaseDim(targetDirs, 1);

	cout("\\e\n");

	if(!NoShutdownFlag)
	{
		coExecute("shutdown -s -t 30");
	}
}
