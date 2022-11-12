#include "C:\Factory\Common\all.h"

static void CComSw(char *source, char *incPtn, uint lineno) // lineno: (1, 2, 3...), 0 のとき、コメント外さない
{
	autoList_t *lines = readLines(source);
	char *line;
	uint index;

	foreach (lines, line, index)
	{
		if (lineExp("<>//**//", line) && strstr(line, incPtn))
		{
			if (!startsWith(line, "/*"))
			{
				line = insertLine(line, 0, "/*");
				setElement(lines, index, (uint)line);
			}
			if (lineno && !--lineno)
			{
				eraseLine(line, 2);
			}
		}
	}
	writeLines(source, lines);
	releaseDim(lines, 1);
}
int main(int argc, char **argv)
{
	CComSw(getArg(0), getArg(1), toValue(getArg(2)));
}
