/*
	> sd

		カレントディレクトリを適当な名前（番号）で保存する。

	> sd NAME

		カレントディレクトリを NAME で保存する。

	> sd NAME DIR

		DIR を NAME で保存する。

	----

	保存したディレクトリへは ld NAME で移動する。
*/

#include "C:\Factory\Common\all.h"
#include "libs\SaveDir.h"

static char *GetAutoName(void)
{
	char *sno = strx("0");

	createFileIfNotExist(SAVE_FILE);

	{
		autoList_t *lines = readLines(SAVE_FILE);
		char *line;
		uint index;

		for (; ; )
		{
			for (index = 0; index < getCount(lines); index += 2)
				if (!_stricmp(sno, getLine(lines, index)))
					break;

			if (index == getCount(lines))
				break;

			sno = xcout("%u", toValue_x(sno) + 1);
		}
	}

	return sno;
}
static char *GetAutoDir(void)
{
	return getCwd();
}
static int SSD_IsAutoName(char *line)
{
	return lineExp("<1,,09>\n<>", line);
}
static sint SSD_Comp(uint v1, uint v2)
{
	char *a = (char *)v1;
	char *b = (char *)v2;
	sint ret;

	{
		sint r1 = SSD_IsAutoName(a) ? 1 : 0;
		sint r2 = SSD_IsAutoName(b) ? 1 : 0;

		ret = r1 - r2;

		if (ret)
			return ret;
	}

	ret = strcmp(a, b);
	return ret;
}
static void SortSaveData(autoList_t *lines)
{
	uint index;

	for (index = 0; index < getCount(lines); index++)
		setElement(lines, index, (uint)addLine_x(getLine(lines, index), addLine_x(strx("\n"), (char *)desertElement(lines, index + 1))));

	rapidSort(lines, SSD_Comp);

	for (index = getCount(lines); index; index--)
	{
		char *p = ne_strchr(getLine(lines, index - 1), '\n');

		insertElement(lines, index, (uint)strx(p + 1));
		*p = '\0';
	}
}
int main(int argc, char **argv)
{
	char *name = NULL;
	char *dir  = NULL;

	name = nnNextArg();
	dir  = nnNextArg();

	if (!name)
		name = GetAutoName(); // g

	if (!dir)
		dir = GetAutoDir(); // g

	line2JToken(name, 1, 1);
	errorCase(m_isEmpty(name));

	dir = makeFullPath(dir);
//	errorCase(!existDir(dir));

	mkAppDataDir();
	createFileIfNotExist(SAVE_FILE);

	{
		autoList_t *lines = readLines(SAVE_FILE);
		char *line;
		uint index;

		for (index = 0; index < getCount(lines); index += 2)
			if (!_stricmp(name, getLine(lines, index)))
				break;

		if (index < getCount(lines))
		{
			strzp((char **)directGetPoint(lines, index + 0), name);
			strzp((char **)directGetPoint(lines, index + 1), dir);
		}
		else
		{
			addElement(lines, (uint)strx(name));
			addElement(lines, (uint)strx(dir));
		}
		SortSaveData(lines);
		writeLines_cx(SAVE_FILE, lines);
	}

	memFree(dir);
}
