/*
	PipeLogger.exe [/DT ログ出力DIR | /UDT ログ出力DIR | /B ログ出力ファイルベース名 | /BH ログ出力ファイルベース名 | ログ出力ファイル]

		/BH ... 1時間毎にログ出力ファイル名を更新する。

	- - -

	使い方

		任意のコマンド | PipeLogger

		と、実行することで任意のコマンドの標準出力を表示しつつ、ログファイルに書き出せる。
		各行頭にタイムスタンプが付く。
*/

#include "C:\Factory\Common\all.h"

#define LOGLINELENMAX 1000000
#define LOGFILE_CHANGE_PERIOD_SEC 3600

static char *LogFile;
static char *LogFileBase = NULL; // NULL == ログ出力ファイル名の更新を行わない。
static uint LogFileNextChangeTime; // 次にログ出力ファイル名の更新を行う時刻
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
	line2JLine(line, 1, 0, 1, 1); // 表示のため
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

	cout("ログファイル：%s\n", LogFile);

	LogFp = fileOpen(LogFile, "wt");

	WrLog("ログ開始");

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
	WrLog("ログ終了");

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

	errorCase_m(hasArgs(2), "不明なコマンド引数");

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
