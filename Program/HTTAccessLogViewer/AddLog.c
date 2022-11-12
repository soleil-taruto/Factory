#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\csvStream.h"

static char *HTTDir;
static char *LogDir;
static int OutputAndDelete;

static char *GetCaptureFile(void)
{
	static char *file;

	if (!file)
		file = combine(getSelfDir(), "Capture.exe");

	return file;
}
static uint GetDateByStamp(char *stamp)
{
	uint date;

	stamp = strxl(stamp, 10);
	eraseChar(stamp + 7);
	eraseChar(stamp + 4);

	date = toValue(stamp);

	memFree(stamp);
	return date;
}
static char *GetWFileByDate(uint date)
{
	return combine_cx(LogDir, xcout("%08u.log", date));
}
static void DistributeToLog(char *rFile)
{
	FILE *rfp = fileOpen(rFile, "rb");
	FILE *wfp = NULL;
	uint wDate = UINTMAX;

	for (; ; )
	{
		autoList_t *row = readCSVRow(rfp);
		uint date;

		if (!row)
			break;

		date = GetDateByStamp(getLine(row, 0));

		if (wDate != date)
		{
			if (wfp)
				fileClose(wfp);

			wfp = fileOpen_xc(GetWFileByDate(date), "ab");
		}
		writeCSVRow_x(wfp, row);
	}
	fileClose(rfp);

	if (wfp)
		fileClose(wfp);
}
static void AddLog(void)
{
	char *midFile = makeTempPath(NULL);

	coExecute_x(xcout("START \"\" /B /WAIT \"%s\"%s \"%s\" \"%s\"", GetCaptureFile(), OutputAndDelete ? " /OAD" : "", HTTDir, midFile));

	LOGPOS();

	DistributeToLog(midFile);

	LOGPOS();

	removeFile(midFile);
	memFree(midFile);
}
int main(int argc, char **argv)
{
readArgs:
	if (argIs("/OAD"))
	{
		LOGPOS();
		OutputAndDelete = 1;
		goto readArgs;
	}

	HTTDir = nextArg();
	LogDir = nextArg();

	errorCase(!existDir(HTTDir));
	errorCase(!existDir(LogDir));

	AddLog();
}
