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
			!_stricmp(dir, "C:\\$SysReset") || // for 10 ゴミ
			!_stricmp(dir, "C:\\Windows.old") || // for 10 ゴミ
			!_stricmp(dir, "C:\\MSOCache") || // for 7, 10 Office 2010

			dir[3] == '_' || m_isdecimal(dir[3]) || // _ 0～9 で始まるフォルダは対象外

//			!_stricmp(dir, "C:\\huge") || // del @ 2017.12.16

			!_stricmp(dir, "C:\\$WinREAgent") || // add @ 2021.3.24

			0
			)
		{
			cout("★除外：%s\n", dir);
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
//		addElement(retDirs, (uint)xcout("C:\\Users\\%s\\Documents", userName)); // 権限が無いとか言われる。
		addElement(retDirs, (uint)xcout("C:\\Users\\%s\\Desktop", userName));
	}
	else // ? XP
	{
		addElement(retDirs, (uint)xcout("C:\\Documents and Settings\\%s\\Favorites", userName));
		addElement(retDirs, (uint)xcout("C:\\Documents and Settings\\%s\\My Documents", userName));
		addElement(retDirs, (uint)xcout("C:\\Documents and Settings\\%s\\デスクトップ", userName));
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
		errorCase(findJLineICase(&followDirs, dir) < getCount(&followDirs)); // ? 重複した。
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
			cout("このフォルダはターゲットです。-> 削除しない。\n");
			continue;
		}
		cout("このフォルダはターゲットではありません。-> 削除する。\n");

		fp = fileOpen(BATCH_BACKUP, "wt");

//		writeLine(fp, "SET COPYCMD=");
		writeLine(fp, line = xcout("RD /S /Q \"%s\"", destDir)); memFree(line);
//		writeLine(fp, line = xcout("ROBOCOPY.EXE \"%s\" \"%s\" /MIR", targetDir, destDir)); memFree(line);
//		writeLine(fp, "ECHO ERRORLEVEL=%ERRORLEVEL%");

		fileClose(fp);

		cout("ロストしたフォルダを削除しています...\n");

		execute(line = xcout("START /B /WAIT CMD /C %s", BATCH_BACKUP)); memFree(line);

		cout("ロストしたフォルダを削除しました。\n");
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

		cout("コピーしています...\n");

		execute(line = xcout("START /B /WAIT CMD /C %s", BATCH_BACKUP)); memFree(line);

		cout("コピーしました。\n");
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

	// 外部コマンド存在確認
	{
		errorCase(!existFile(FILE_TOOLKIT_EXE));
	}

	NoShutdownFlag = argIs("/-S");

	if(!NoShutdownFlag)
	{
		cout("********************************\n");
		cout("** 終了後シャットダウンします **\n");
		cout("********************************\n");
	}
	cout("+----------------------------------+\n");
	cout("| バックアップ先は P ドライブです。|\n");
	cout("+----------------------------------+\n");
	cout("+---------------------------------------------+\n");
	cout("| メーラーなど、動作中のアプリを閉じて下さい。|\n");
	cout("+---------------------------------------------+\n");
//	cout("+---------------------------------------+\n");
//	cout("| ウィルス対策ソフトを無効にして下さい。|\n");
//	cout("+---------------------------------------+\n");

	{
		cout("続行？\n");

		if(clearGetKey() == 0x1b)
			termination(0);

		cout("続行します。\n");
	}

	strDestDrv = strx("P:\\");
	errorCase_m(!existDir(strDestDrv), "バックアップ先の P ドライブが存在しません。");
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

	cout("出力先: %s\n", destDir);
	cout("\n");

	targetDirs = GetTargetDirs();
	CheckTargetDirs(targetDirs);

	createDirIfNotExist(destDir);
	coExecute_x(xcout("Compact.exe /C \"%s\"", destDir));
//	coExecute_x(xcout("Compact.exe /C /S:\"%s\"", destDir));

	addCwd(destDir);
	coExecute("DEL /Q *"); // destDir の直下のファイルを全て削除する。
	EraseLostTargetDirs(targetDirs);
	BackupDirs(targetDirs);
	unaddCwd();

	// _ 0～9 で始まる名前はバックアップ対象外なので、destDir の直下は _ 0～9 で始めておけば重複しないはず。

#if 0 // 抑止 @ 2020.10.24
	// Toolkit が USB HDD への書き込みエラーを起こしたので、書き込み先をシステムドライブにして様子を見る。@ 2018.3.5
	// ハッシュリスト作成
	{
		char *midDir = makeTempDir("Backup_Hash_txt_mid");

		coExecute_x(xcout(FILE_TOOLKIT_EXE " /SHA-512-128 %s %s\\_Hash.txt", destDir, midDir));

		coExecute_x(xcout("COPY /Y %s\\_Hash.txt %s\\_Hash.txt", midDir, destDir));
		coExecute_x(xcout("COPY /Y %s\\_Hash.txt C:\\tmp\\Backup_Hash.txt", midDir));

		recurRemoveDir_x(midDir);
	}
#endif

	// Backup.bat との連携
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
