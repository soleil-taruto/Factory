/*
	PipeLogger.exe [/DT ���O�o��DIR | /UDT ���O�o��DIR | /B ���O�o�̓t�@�C���x�[�X�� | /BH ���O�o�̓t�@�C���x�[�X�� | ���O�o�̓t�@�C��]

		/BH ... 1���Ԗ��Ƀ��O�o�̓t�@�C�������X�V����B

	- - -

	�g����

		�C�ӂ̃R�}���h | PipeLogger

		�ƁA���s���邱�ƂŔC�ӂ̃R�}���h�̕W���o�͂�\�����A���O�t�@�C���ɏ����o����B
		�e�s���Ƀ^�C���X�^���v���t���B
*/

#include "C:\Factory\Common\all.h"

#define LOGLINELENMAX 1000000
#define LOGFILE_CHANGE_PERIOD_SEC 3600

static char *LogFile;
static char *LogFileBase = NULL; // NULL == ���O�o�̓t�@�C�����̍X�V���s��Ȃ��B
static uint LogFileNextChangeTime; // ���Ƀ��O�o�̓t�@�C�����̍X�V���s������
static FILE *LogFp;

static char *GetLogFile(char *prefix, char *prefix2, char *prefix3, char *prefix4)
{
	char *file = xcout("%s%s%s%s%s000.log", prefix, prefix2, prefix3, prefix4, c_makeCompactStamp(NULL));

	file = toCreatablePath(file, 999);
	return file;
}
static char *GetNextLogFile(void)
{
	return GetLogFile(LogFileBase, "", "", "");
}
static void WrLog(char *line)
{
	line = xcout("[%s] %s", c_makeJStamp(NULL, 0), line);
	writeLine(LogFp, line);
	fflush(LogFp);
	line2JLine(line, 1, 0, 1, 1); // �\���̂���
	cout("%s\n", line);
	memFree(line);

	if (LogFileBase)
	{
		uint currTime = now();

		if (LogFileNextChangeTime < currTime)
		{
			memFree(LogFile);
			LogFile = GetNextLogFile();
			LogFileNextChangeTime = currTime + LOGFILE_CHANGE_PERIOD_SEC;

			fileClose(LogFp);
			LogFp = fileOpen(LogFile, "wt");
		}
	}
}
static void PipeLogger(void)
{
	char *line = memAlloc(LOGLINELENMAX + 1);
	uint wPos = 0;

	cout("���O�t�@�C���F%s\n", LogFile);

	LogFp = fileOpen(LogFile, "wt");

	WrLog("���O�J�n");

	for (; ; )
	{
		int chr = fgetc(stdin);

		if (chr == EOF)
			break;

		if (chr == '\r')
		{
			// noop
		}
		else if (chr == '\n')
		{
			line[wPos] = '\0';
			WrLog(line);
			wPos = 0;
		}
		else
		{
			if (LOGLINELENMAX <= wPos)
			{
				line[wPos] = '\0';
				WrLog(line);
				wPos = 0;
			}
			line[wPos++] = chr;
		}
	}
	if (wPos)
	{
		line[wPos] = '\0';
		WrLog(line);
	}
	WrLog("���O�I��");

	fileClose(LogFp);
	LogFp = NULL;

	memFree(line);
}
int main(int argc, char **argv)
{
	if (argIs("/DT"))
	{
		char *dir = nextArg();

		LogFile = GetLogFile(dir, "\\", "", "");

		PipeLogger();
		return;
	}
	if (argIs("/UDT"))
	{
		char *dir = nextArg();

		LogFile = GetLogFile(dir, "\\", getEnvLine("USERNAME"), "_");

		PipeLogger();
		return;
	}
	if (argIs("/B"))
	{
		char *fileBase = nextArg();

		LogFile = GetLogFile(fileBase, "", "", "");

		PipeLogger();
		return;
	}
	if (argIs("/BH"))
	{
		LogFileBase = nextArg();

		LogFile = GetNextLogFile();
		LogFileNextChangeTime = now() + LOGFILE_CHANGE_PERIOD_SEC;

		PipeLogger();
		return;
	}

	errorCase_m(hasArgs(2), "�s���ȃR�}���h����");

	if (hasArgs(1))
	{
		LogFile = nextArg();

		PipeLogger();
		return;
	}

	{
		LogFile = getOutFile_x(xcout("PipeLog_%s.txt", c_makeCompactStamp(NULL)));

		PipeLogger();

		openOutDir();
		return;
	}
}
