/*
	LiteBackup.exe [�Ώۃf�B���N�g��]
*/

#include "C:\Factory\Common\all.h"

/*
	�i�[��t�H���_
	���݂��Ȃ��ꍇ�͎蓮�ō쐬���邱�ƁB
	���󔒕s��
*/
#define BACKUP_DIR "C:\\temp\\Backup"

/*
	rum.exe �ւ̃p�X
*/
#define RUM_EXE "C:\\Factory\\Tools\\rum.exe"

static void LiteBackup(char *targDir)
{
	errorCase(!existDir(BACKUP_DIR)); // ���S�̂��ߎ蓮�ō쐬���邱�ƁB
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
