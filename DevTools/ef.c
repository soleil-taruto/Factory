#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	autoList_t *files;
	autoList_t *selfiles;

	cout("+-----------------------------+\n");
	cout("| ファイルリストを編集します。|\n");
	cout("+-----------------------------+\n");
	sleep(500); // 見えるように

	createFileIfNotExist(FOUNDLISTFILE);
	files = readLines(FOUNDLISTFILE);
	selfiles = selectLines(files);

	if (getCount(selfiles))
	{
		if (isSameLines(files, selfiles, 0))
		{
			cout("更新不要\n");
		}
		else
		{
			writeLines(FOUNDLISTFILE, selfiles);
			cout("更新OK\n");
		}
	}
	releaseDim(files, 1);
	releaseDim(selfiles, 1);
}
