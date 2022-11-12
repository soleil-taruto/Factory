/*
	Template.exe /L

		リスト表示

	Template.exe テンプレート名 識別子

		テンプレート作成

	例：

		template dc GoldBlend
*/

#include "C:\Factory\Common\all.h"

#define TEMPLATE_DIR "C:\\Factory\\DevTools\\TemplateFiles"
#define ESCAPE_PTN "\1"
#define DESCRIPTION_PREFIX "/// "

static char *GetAllUpper(char *userParam)
{
	char *ret = strx("");
	char *p;

	for (p = userParam; *p; p++)
		if (m_isupper(*p))
			ret = addChar(ret, *p);

	return ret;
}
static void MakeTemplate(char *templateName, char *userParam)
{
	char *templateFile = xcout(TEMPLATE_DIR "\\%s.txt", templateName);
	autoList_t *lines;
	char *line;
	uint index;
	char *initial = xcout("%c", userParam[0]);
	char *allUpper = GetAllUpper(userParam);

	errorCase(!existFile(templateFile));

	lines = readLines(templateFile);

	if (getCount(lines) && startsWith(getLine(lines, 0), DESCRIPTION_PREFIX))
	{
		do
		{
			memFree((char *)desertElement(lines, 0));
		}
		while (getCount(lines) && !*getLine(lines, 0));
	}
	foreach (lines, line, index)
	{
		line = replaceLine(line, "$$", ESCAPE_PTN, 0);
		line = replaceLine(line, "$I", initial, 1);
		line = replaceLine(line, "$U", allUpper, 1);
		line = replaceLine(line, "$P", userParam, 1);
		line = replaceLine(line, ESCAPE_PTN, "$", 0);

		setElement(lines, index, (uint)line);
	}
	viewLines(lines);
	releaseDim(lines, 1);
	memFree(templateFile);
	memFree(initial);
}
int main(int argc, char **argv)
{
	if (argIs("/L"))
	{
		autoList_t *files = lsFiles(TEMPLATE_DIR);
		char *file;
		uint index;

		//eraseParents(files); // del

		foreach (files, file, index)
			toUpperLine(file);

		rapidSortLines(files);

		foreach (files, file, index)
		{
			if (!_stricmp("txt", getExt(file)))
			{
				char *templateName = changeExt(getLocal(file), "");
				char *description = readFirstLine(file);

				if (startsWith(description, DESCRIPTION_PREFIX))
					eraseLine(description, 4);
				else
					strzp(&description, "説明はありません。");

				cout("%s\t%s\n", templateName, description);
				memFree(templateName);
				memFree(description);
			}
		}
		releaseDim(files, 1);
		return;
	}
	MakeTemplate(getArg(0), getArg(1));
}
