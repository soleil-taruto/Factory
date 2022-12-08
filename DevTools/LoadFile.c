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
			cout("既にロードされているファイルはセーブされていません。\n");
			cout("保存時に復元されます。\n");
			cout("保存しない場合、編集されたファイルは退避します。\n");
			cout("####################################################\n");

			origLoadedFile = makeTempPath(NULL);
			copyFile(loadedFile, origLoadedFile);
		}
		copyFile(savedFile, loadedFile);

		cout("ロードしました。\n");
		cout("保存するには何かキーを押して下さい。\n");
		cout("エスケープキーを押すと保存せずに終了します。\n");

		if (clearGetKey() == 0x1b) // ? 保存しない
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

	errorCase(!handleWaitForMillis(mtx, 0)); // ? 多重起動

	LoadFile(getArg(0), getArg(1));

	mutexRelease(mtx);
	handleClose(mtx);
}
