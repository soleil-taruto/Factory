/*
	ChooseOneLine.exe ���̓t�@�C�� [/ER �ŋ߂̏o�͂̃L���b�V���t�@�C��] (/L | /R | /M �����s��\����������b�� | /I �C���f�b�N�X)

		/L ... �S�s�\������B
		/R ... �����_���ɑI�΂ꂽ�P�s��\������B
		/M ... ���ԂőI�΂ꂽ�P�s��\������B
		/I ... �w�肳�ꂽ�P�s��\������B
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\CRRandom.h"

static autoList_t *Lines;
static char *RecentlyLinesFile;

static void PrintLine(uint index)
{
	char *line = getLine(Lines, index);

	cout("%s\n", line);

	if (RecentlyLinesFile)
		addLine2File(RecentlyLinesFile, line);
}
static void DoRandom(void)
{
	PrintLine(mt19937_rnd(getCount(Lines)));
}
static void DoModTime(uint timePerLine)
{
	errorCase(!m_isRange(timePerLine, 1, IMAX));

	PrintLine((uint)((time(NULL) / timePerLine) % getCount(Lines)));
}
int main(int argc, char **argv)
{
	mt19937_initCRnd();

	Lines = readResourceLines(nextArg());
	Lines = autoDistinctLines(Lines);

	errorCase(!getCount(Lines));

	if (argIs("/ER"))
	{
		autoList_t *rcntLines;
		char *line;
		uint index;

		RecentlyLinesFile = nextArg();
		createFileIfNotExist(RecentlyLinesFile);

		rcntLines = readLines(RecentlyLinesFile);

		while (getCount(Lines) / 2 < getCount(rcntLines))
			memFree((char *)desertElement(rcntLines, 0));

		writeLines(RecentlyLinesFile, rcntLines);

		foreach (Lines, line, index)
			if (findLine(rcntLines, line) < getCount(rcntLines))
				*line = '\0';

		trimLines(Lines);
		errorCase(!getCount(Lines)); // 2bs ��ɂȂ�Ȃ��͂��B

		releaseDim(rcntLines, 1);
	}

	if (argIs("/L"))
	{
		char *line;
		uint index;

		foreach (Lines, line, index)
			cout("%s\n", line);

		return;
	}
	if (argIs("/R"))
	{
		DoRandom();
		return;
	}
	if (argIs("/M"))
	{
		DoModTime(toValue(nextArg()));
		return;
	}
	if (argIs("/I"))
	{
		uint index = toValue(nextArg());

		PrintLine(index);
		return;
	}
	error_m("�s���ȃR�}���h����");
}
