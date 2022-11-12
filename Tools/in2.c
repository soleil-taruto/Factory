/*
	> in

		選択したサブディレクトリにカレントディレクトリを移動する。
*/

#include "C:\Factory\Common\all.h"

#define BATCH_FILE "C:\\Factory\\tmp\\IntoDir.bat"

int main(int argc, char **argv)
{
	autoList_t *dirs = lsDirs(".");
	char *selDir;

	removeFileIfExist(BATCH_FILE);

	eraseParents(dirs);
	selDir = selectLine(dirs);

	if (selDir)
		writeOneLine_cx(BATCH_FILE, xcout("CD \"%s\"", selDir));

	releaseDim(dirs, 1);
}
