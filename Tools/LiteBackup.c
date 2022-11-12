/*
	LiteBackup.exe [対象ディレクトリ]
*/

#include "C:\Factory\Common\all.h"

/*
	格納先フォルダ
	存在しない場合は手動で作成すること。
	★空白不可
*/
#define BACKUP_DIR "C:\\temp\\Backup"

/*
	rum.exe へのパス
*/
#define RUM_EXE "C:\\Factory\\Tools\\rum.exe"

static void LiteBackup(char *targDir)
{
	errorCase(!existDir(BACKUP_DIR)); // 安全のため手動で作成すること。
	targDir = makeFullPath(targDir);
	errorCase(!existDir(targDir));

	recurClearDir(BACKUP_DIR);
	copyDir(targDir, BACKUP_DIR);

	coExecute_x(xcout(RUM_EXE " /C \"%s\"", targDir));
	coExecute(RUM_EXE " /Q " BACKUP_DIR);
	coExecute(RUM_EXE " /C-");

	memFree(targDir);
}
int main(int argc, char **argv)
{
	char *dir;

	cout("+-------------+\n");
	cout("| LITE-BACKUP |\n");
	cout("+-------------+\n");

	if (hasArgs(1))
		dir = nextArg();
	else
		dir = c_dropDir();

	LiteBackup(dir);
}
