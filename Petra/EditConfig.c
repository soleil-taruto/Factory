/*
	EditConfig.exe �ݒ�t�@�C�� �ҏW�Ώۍs�̒��O�̍s �ҏW�Ώۍs�ɏ㏑�����镶����
*/

#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	char *file;
	char *mLine;
	char *owLine;
	autoList_t *lines;
	char *line;
	uint index;

	LOGPOS();

	file = nextArg();
	mLine = nextArg();
	owLine = nextArg();

	errorCase(m_isEmpty(file));
	errorCase(!existFile(file));
	// mLine
	// owLine

	lines = readLines(file);

	foreach (lines, line, index)
	{
		if (!strcmp(line, mLine))
		{
			index++; // �ҏW�Ώۂ͎��̍s
			break;
		}
	}
	errorCase(getCount(lines) <= index); // ? �ҏW�Ώۂ̍s�����݂��Ȃ��B

	strzp((char **)directGetPoint(lines, index), owLine);

	writeLines_cx(file, lines);

	LOGPOS();
}
