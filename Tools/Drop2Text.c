/*
	Drop2Text.exe [/F]

		/F ... フォルダがドロップされた場合、その配下のファイルを追加する。
*/

#include "C:\Factory\Common\all.h"

static int FileOnlyMode;

int main(int argc, char **argv)
{
	autoList_t *paths = newList();

	if (argIs("/F"))
	{
		FileOnlyMode = 1;
	}

	for (; ; )
	{
		char *path = dropPath();

		if (!path)
			break;

		if (FileOnlyMode && existDir(path))
		{
			autoList_t *files = lssFiles(path);
			sortJLinesICase(files);
			addElements_x(paths, files);
		}
		else
		{
			addElement(paths, (uint)path);
		}
	}
	writeLines_cx("C:\\Factory\\tmp\\Drop2Text_Last.txt", editLines_x(paths));
}
