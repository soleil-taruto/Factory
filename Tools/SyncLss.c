#include "C:\Factory\Common\all.h"
#include "C:\Factory\OpenSource\md5.h"

static char *GetCommonPathPrefixWithStar(autoList_t *paths, char *exceptPath)
{
	char *pfx = NULL;
	char *path;
	int star = 0;
	uint index;

	foreach (paths, path, index)
	{
		if (!mbs_stricmp(path, exceptPath))
			continue;

		if (!pfx)
		{
			pfx = strx(path);
		}
		else
		{
			uint i;

			for (i = 0; pfx[i] && m_toupper(pfx[i]) == m_toupper(path[i]); i++);

			if (!pfx[i])
			{
				if (path[i])
					star = 1;
			}
			else
			{
				pfx[i] = '\0';
				star = 1;
			}
		}
	}
	errorCase(!pfx);

	line2JLine(pfx, 1, 0, 0, 1); // 全角文字の途中で区切られるかもしれないので

	if (star)
		pfx = addChar(pfx, '*');

	return pfx;
}
static void SyncLss(void)
{
	autoList_t *files = readLines(FOUNDLISTFILE);
	char *file;
	uint index;
	autoList_t *lines = newList();
	char *selLine;
	char *selFile;

isJChar(0); // res: C:\Factory\Resource\JIS0208.txt 対策

	foreach (files, file, index)
		addElement(lines, (uint)xcout("%s %s", md5_makeHexHashFile(file), file));

	selLine = selectLine(lines);

	if (!selLine)
		termination(0);

	selFile = strx(selLine + 33);

	cout("< %s\n", selFile);
	cout("> %s\n", GetCommonPathPrefixWithStar(files, selFile)); // g

	for (; ; )
	{
		const char *CFM_PTN = "SYNC LSS";
		char *cfmInput;

		cout("### 確認のため %s と入力してね。### (ignore case)\n", CFM_PTN);
		cfmInput = coInputLine();

		if (!*cfmInput)
		{
			memFree(cfmInput);
			termination(0);
		}
		if (!_stricmp(cfmInput, CFM_PTN)) // ? 一致した。
		{
			memFree(cfmInput);
			break;
		}
		memFree(cfmInput);
	}

	foreach (files, file, index)
	{
		if (!mbs_stricmp(file, selFile))
			continue;

		cout("< %s\n", selFile);
		cout("> %s\n", file);

		semiRemovePath(file);

		copyFile(selFile, file);
	}
	releaseDim(files, 1);
	releaseDim(lines, 1);
	memFree(selLine);
	memFree(selFile);
}
int main(int argc, char **argv)
{
	SyncLss();
}
