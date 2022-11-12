/*
	mask.exe 開始列 長さ

		開始列  ...  0 ～ IMAX
		長さ    ...  1 ～ IMAX

	----
	使用例

	TYPE C:\temp\Backup.log | Grape "ECHO ERRORLEVEL=" | mask 1 24 | TypeGroupBy
*/

#include "C:\Factory\Common\all.h"

static uint MaskStartIndex;
static uint MaskLen;

static void RLFS_LineAction(char *line)
{
	uint lineLen = strlen(line);
	uint count;

	for (count = 0; count < MaskLen; count++)
	{
		uint index = MaskStartIndex + count;

		if (lineLen <= index)
			break;

		line[index] = '*';
	}
	cout("%s\n", line);
}
static void ReadLinesFromStdin(void)
{
	char *line;

	while (line = readLine(stdin))
	{
		RLFS_LineAction(line);
		memFree(line);
	}
}
int main(int argc, char **argv)
{
	MaskStartIndex = toValue(nextArg());
	MaskLen        = toValue(nextArg());

	errorCase(!m_isRange(MaskStartIndex, 0, IMAX));
	errorCase(!m_isRange(MaskLen,        1, IMAX));

	ReadLinesFromStdin();
}
