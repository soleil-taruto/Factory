#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\csvStream.h"

#define MUTEX_LOG "{4e08b31a-9280-4ee0-9f9a-2ef462589893}" // shared_uuid@ign このプログラムはもう使わない。

static char *HTTDir;
static char *WFile;
static int OutputAndDelete;

static void AddLogFile(autoList_t *lines, char *logFile)
{
	if (existFile(logFile))
		addElements_x(lines, readLines(logFile));
}
static void RemoveLogFile(char *logFile)
{
	removeFileIfExist(logFile);
}

// ---- log line ----

static char *Stamp;
static char *ClientIP;
static char *Method;
static char *Path;
static char *Host;
static char *UserAgent;
static char *Domain;
static char *TargetPath;
static char *Status;

static void ClearLogLine(void)
{
	strzp(&Stamp,      "");
	strzp(&ClientIP,   "");
	strzp(&Method,     "");
	strzp(&Path,       "");
	strzp(&Host,       "");
	strzp(&UserAgent,  "");
	strzp(&Domain,     "");
	strzp(&TargetPath, "");
	strzp(&Status,     "");
}
static void WriteLogLine(FILE *fp)
{
	autoList_t *row = newList();

	// 2bs
	{
		line2JLine(Stamp,      1, 0, 0, 1);
		line2JLine(ClientIP,   1, 0, 0, 1);
		line2JLine(Method,     1, 0, 0, 1);
		line2JLine(Path,       1, 0, 0, 1);
		line2JLine(Host,       1, 0, 0, 1);
		line2JLine(UserAgent,  1, 0, 0, 1);
		line2JLine(Domain,     1, 0, 0, 1);
		line2JLine(TargetPath, 1, 0, 0, 1);
		line2JLine(Status,     1, 0, 0, 1);
	}

	addElement(row, (uint)Stamp);
	addElement(row, (uint)ClientIP);
	addElement(row, (uint)Method);
	addElement(row, (uint)Path);
	addElement(row, (uint)Host);
	addElement(row, (uint)UserAgent);
	addElement(row, (uint)Domain);
	addElement(row, (uint)TargetPath);
	addElement(row, (uint)Status);

	writeCSVRow(fp, row);

	releaseAutoList(row);
}

// ---- log line / parse ----

static void ParseLogLine(char *line)
{
	char *p;
	char *q;

	q = line;
	p = q;

	q = ne_strchr(q, ':') + 1;
	q = ne_strchr(q, ' ');

	*q = '\0';
	strzp(&Stamp, p);
	q++;
	p = q;

	q = ne_strstr(q, " \""); // 2 bytes

	*q = '\0';
	strzp(&ClientIP, p);
	q += 2;
	p = q;

	q = ne_strchr(q, ' ');

	*q = '\0';
	strzp(&Method, p);
	q++;
	p = q;

	q = ne_strstr(q, "\" Host:\""); // 8 bytes

	*q = '\0';
	strzp(&Path, p);
	q += 8;
	p = q;

	q = ne_strstr(q, "\" User-Agent:\""); // 14 bytes

	*q = '\0';
	strzp(&Host, p);
	q += 14;
	p = q;

	q = strchr(p, '\0');

	if (p < q && q[-1] == '"')
		q--;

	*q = '\0';
	strzp(&UserAgent, p);
}
static void ParseLogLine2(char *line)
{
	char *p;
	char *q;

	q = ne_strchr(line, '"') + 1;
	p = q;

	q = ne_strstr(q, "\" Target-Path:\""); // 15 bytes

	*q = '\0';
	strzp(&Domain, p);
	q += 15;
	p = q;

	q = strchr(p, '\0');

	if (p < q && q[-1] == '"')
		q--;

	*q = '\0';
	strzp(&TargetPath, p);
}
static void ParseLogLine3(char *line)
{
	strzp(&Status, ne_strchr(line, ' ') + 1);
}

// ----

static void CaptureMain(void)
{
	autoList_t *lines = newList();
	uint index;
	FILE *fp = fileOpen(WFile, "ab");

	addCwd(HTTDir);
	{
		AddLogFile(lines, "AccessLog0.dat");
		AddLogFile(lines, "AccessLog.dat");
	}
	unaddCwd();

	for (index = 0; index < getCount(lines); index++)
	{
		char *line = getLine(lines, index);

		if (m_isdecimal(line[0]))
		{
			char *line2 = NULL;
			char *line3 = NULL;

			if (refLine(lines, index + 1)[0] == 'D')
			{
				line2 = getLine(lines, ++index);

				if (refLine(lines, index + 1)[0] == 'S')
					line3 = getLine(lines, ++index);
			}
			ClearLogLine();
			ParseLogLine(line);

			if (line2)
				ParseLogLine2(line2);

			if (line3)
				ParseLogLine3(line3);

			WriteLogLine(fp);
		}
	}
	releaseDim(lines, 1);
	fileClose(fp);

	if (OutputAndDelete)
	{
		addCwd(HTTDir);
		{
			RemoveLogFile("AccessLog0.dat");
			RemoveLogFile("AccessLog.dat");
		}
		unaddCwd();
	}
}
int main(int argc, char **argv)
{
	uint mtx;

readArgs:
	if (argIs("/OAD"))
	{
		LOGPOS();
		OutputAndDelete = 1;
		goto readArgs;
	}

	HTTDir = nextArg();
	WFile  = nextArg();

	errorCase(m_isEmpty(HTTDir));
	errorCase(m_isEmpty(WFile));

	HTTDir = makeFullPath(HTTDir);
	WFile  = makeFullPath(WFile);

	errorCase(!existDir(HTTDir));

	createFileIfNotExist(WFile);

	mtx = mutexLock(MUTEX_LOG);
	{
		CaptureMain();
	}
	mutexUnlock(mtx);
}
