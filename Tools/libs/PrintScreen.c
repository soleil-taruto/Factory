#include "PrintScreen.h"

autoList_t *PrintScreen(void)
{
	char *dir = makeTempDir(NULL);
	char *fileBase;
	uint screenNo;
	autoList_t *bmps = newList();

	errorCase(!existFile(FILE_TOOLKIT_EXE)); // 外部コマンド存在確認

	fileBase = combine(dir, "Screen_");

	coExecute_x(xcout(FILE_TOOLKIT_EXE " /PRINT-SCREEN %s", fileBase));

	for (screenNo = 1; ; screenNo++)
	{
		char *file = xcout("%s%02u.bmp", fileBase, screenNo);

		if (!existFile(file))
		{
			memFree(file);
			break;
		}
		addElement(bmps, (uint)readBinary(file));
		removeFile(file);
		memFree(file);
	}
	removeDir(dir);
	memFree(dir);
	memFree(fileBase);
	return bmps;
}
void PrintScreen_Dir(char *dir)
{
	autoList_t *bmps = PrintScreen();
	autoBlock_t *bmp;
	uint index;

	errorCase(!existDir(dir));

	foreach (bmps, bmp, index)
	{
		char *file = combine_cx(dir, xcout("Screen_%02u.bmp", (index + 1)));

		writeBinary(file, bmp);

		memFree(file);
		releaseAutoBlock(bmp);
	}
	releaseAutoList(bmps);
}
