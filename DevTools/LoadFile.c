#include "C:\Factory\Common\all.h"

static int IsSavedFile(char *saveDir, char *loadedFile)
{
	autoList_t *files = lsFiles(saveDir);
	char *file;
	uint index;

	foreach (files, file, index)
		if (isSameFile(file, loadedFile))
			break;

	releaseDim(files, 1);
	return file ? 1 : 0;
}
static char *SelectSavedFile(char *saveDir)
{
	autoList_t *files = lsFiles(saveDir);
	char *file;

	eraseParents(files);
	file = selectLine(files);

	if (file)
		file = combine_cx(saveDir, file);

	return file;
}
static void LoadFile(char *saveDir, char *loadedFile)
{
	char *savedFile;
	char *origLoadedFile = NULL;

	errorCase(!existDir(saveDir));
	errorCase(!existFile(loadedFile) && !creatable(loadedFile));

	savedFile = SelectSavedFile(saveDir);

	if (savedFile)
	{
		if (existFile(loadedFile) && !IsSavedFile(saveDir, loadedFile))
		{
			cout("####################################################\n");
			cout("���Ƀ��[�h����Ă���t�@�C���̓Z�[�u����Ă��܂���B\n");
			cout("�ۑ����ɕ�������܂��B\n");
			cout("�ۑ����Ȃ��ꍇ�A�ҏW���ꂽ�t�@�C���͑ޔ����܂��B\n");
			cout("####################################################\n");

			origLoadedFile = makeTempPath(NULL);
			copyFile(loadedFile, origLoadedFile);
		}
		copyFile(savedFile, loadedFile);

		cout("���[�h���܂����B\n");
		cout("�ۑ�����ɂ͉����L�[�������ĉ������B\n");
		cout("�G�X�P�[�v�L�[�������ƕۑ������ɏI�����܂��B\n");

		if (clearGetKey() == 0x1b) // ? �ۑ����Ȃ�
		{
			if (origLoadedFile && !IsSavedFile(saveDir, loadedFile))
			{
				char *escDir = makeFreeDir();
				char *escFile;

				escFile = combine(escDir, getLocal(loadedFile));
				copyFile(loadedFile, escFile);
				execute_x(xcout("START %s", escDir));

				memFree(escDir);
				memFree(escFile);
			}
		}
		else
		{
			copyFile(loadedFile, savedFile);
		}
		memFree(savedFile);
	}
	if (origLoadedFile)
	{
		copyFile(origLoadedFile, loadedFile);
		removeFile(origLoadedFile);
		memFree(origLoadedFile);
	}
}
int main(int argc, char **argv)
{
	uint mtx = mutexOpen("cerulean.charlotte Factory LoadFile Proc mutex object");

	errorCase(!handleWaitForMillis(mtx, 0)); // ? ���d�N��

	LoadFile(getArg(0), getArg(1));

	mutexRelease(mtx);
	handleClose(mtx);
}
