/*
	mask.exe �J�n�� ����

		�J�n��  ...  0 �` IMAX
		����    ...  1 �` IMAX

	----
	�g�p��

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
