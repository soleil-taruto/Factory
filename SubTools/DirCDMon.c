#include "C:\Factory\Common\all.h"

static int MonTarget = 'D'; // "DF"
static autoList_t *MonDirs;
static autoList_t *LastLists;

static void ShowCDList(autoList_t *lines, int chrFlg)
{
	char *line;
	uint index;

	foreach (lines, line, index)
	{
		cout("%s %c %s\n", c_makeCompactStamp(NULL), chrFlg, line);

#if 0
		if (strlen(getLocal(line)) == 29) // 暫定
		{
			static sint processingCount;

			if (chrFlg == '+')
				processingCount++;
			else
				processingCount--;

			cout("*** PROCESSING %d %c ***\n", processingCount, chrFlg);
		}
#endif
	}
}
static void ShowDirCD(char *monDir, autoList_t *listNew, autoList_t *oldList)
{
	autoList_t *creList = newList();
	autoList_t *delList = newList();

	listNew = copyLines(listNew);
	oldList = copyLines(oldList);

	mergeLines2ICase(listNew, oldList, creList, NULL, delList);

	sortJLinesICase(creList);
	sortJLinesICase(delList);

	ShowCDList(creList, '+');
	ShowCDList(delList, '-');

	releaseDim(listNew, 1);
	releaseDim(oldList, 1);
	releaseAutoList(creList);
	releaseAutoList(delList);
}
static void DirCDMon(void)
{
	char *monDir;
	uint monDir_index;

	for (; ; )
//	while (waitKey(0) != 0x1b) // old
	{
		for (; ; )
		{
			switch (waitKey(2000))
			{
			case 0x00:
				goto endKeyLoop;

			case 0x1b:
				goto endLoop;

			case '+':
				cout("＋\n");
				break;

			case ' ':
				execute("CLS");
				break;
			}
		}
	endKeyLoop:

		foreach (MonDirs, monDir, monDir_index)
		{
			autoList_t *list;

			if (MonTarget == 'D')
				list = cmdDir_lsDirs(monDir);
			else // F
				list = cmdDir_lsFiles(monDir);

			sortJLinesICase(list);

			if (refElement(LastLists, monDir_index))
			{
				autoList_t *lastList = getList(LastLists, monDir_index);

				ShowDirCD(monDir, list, lastList);

				releaseDim(lastList, 1);
			}
			putElement(LastLists, monDir_index, (uint)list);
		}
//		sleep(2000); // -> waitKey
	}
endLoop:;
}
int main(int argc, char **argv)
{
	MonDirs = newList();
	LastLists = newList();

	if (argIs("/F"))
	{
		MonTarget = 'F';
	}
	while (hasArgs(1))
	{
		char *dir = nextArg();

//		dir = makeFullPath(dir); // ネットワークドライブ対応のため、
//		errorCase(!existDir(dir)); // ネットワークドライブ対応のため、
		errorCase(!accessible(dir));

		addElement(MonDirs, (uint)dir);
	}
	if (!getCount(MonDirs))
	{
		addElement(MonDirs, (uint)getCwd());
	}
	DirCDMon();
}
