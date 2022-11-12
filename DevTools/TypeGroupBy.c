/*
	TypeGroupBy.exe [/C] [/-C] [/I] [���̓t�@�C��]

		/C  ... �o�͍s�� (�O���[�v��) ��\������B
		/-C ... �o�͍s�Ɍ�����\�����Ȃ��B
		/I  ... �啶���E����������ʂ��Ȃ��B

		���̓t�@�C�����ȗ�����ƁA�W�����͂���ǂݍ���
*/

#include "C:\Factory\Common\all.h"

static autoList_t *ReadLinesFromStdin(void)
{
	autoList_t *lines = newList();
	char *line;

	while (line = readLine(stdin))
		addElement(lines, (uint)line);

	return lines;
}
int main(int argc, char **argv)
{
	int countMode = 0;
	int noCountMode = 0;
	int ignoreCase = 0;
	autoList_t *lines;
	char *line;
	uint index;
	uint count;
	autoList_t *outLines = newList();

readArgs:
	if (argIs("/C"))
	{
		countMode = 1;
		goto readArgs;
	}
	if (argIs("/-C"))
	{
		noCountMode = 1;
		goto readArgs;
	}
	if (argIs("/I"))
	{
		ignoreCase = 1;
		goto readArgs;
	}
	if (hasArgs(1))
		lines = readLines(nextArg());
	else
		lines = ReadLinesFromStdin();

	if (ignoreCase)
		foreach (lines, line, index)
			toUpperLine(line);

	rapidSortLines(lines);

	for (index = 0; index < getCount(lines); index += count)
	{
		for (count = 0; index + count < getCount(lines); count++)
			if (strcmp(getLine(lines, index), getLine(lines, index + count)))
				break;

		if (noCountMode)
			line = xcout("%s", getLine(lines, index));
		else
			line = xcout("%10u %s", count, getLine(lines, index));

		addElement(outLines, (uint)line);
	}
	releaseDim(lines, 1);
	lines = outLines;

	for (; ; )
	{
		foreach (lines, line, index)
			if (line[0] != ' ')
				break;

		if (line)
			break;

		foreach (lines, line, index)
			eraseChar(line);
	}
	rapidSortLines(lines);

	if (countMode)
		cout("%u\n", getCount(lines));
	else
		foreach (lines, line, index)
			cout("%s\n", line);

	releaseDim(lines, 1);
}
