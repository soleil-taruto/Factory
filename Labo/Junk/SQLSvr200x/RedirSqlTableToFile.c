#include "C:\Factory\Common\all.h"
#include "libs\RedirSqlTableReader.h"

static int CSVMode;

static void ToFile(char *file, char *outFile)
{
	( CSVMode ? RSTR_ToCSVFile : RSTR_ToLinearFile )(file, outFile);
}
static void ToConsole(char *file)
{
	char *outFile = makeTempPath("out");
	FILE *fp;
	char *line;

	ToFile(file, outFile);

	fp = fileOpen(outFile, "rt");

	while (line = readLine(fp))
	{
		cout("%s\n", line);
		memFree(line);
	}
	fileClose(fp);
	removeFile(outFile);
	memFree(outFile);
}
int main(int argc, char **argv)
{
	if (argIs("/C"))
	{
		CSVMode = 1;
	}
	if (argIs("/-H")) // no Header
	{
		RSTR_NoHeaderMode = 1;
	}

	if (hasArgs(2))
	{
		ToFile(getArg(0), getArg(1));
		return;
	}
	if (hasArgs(1))
	{
		ToConsole(nextArg());
		return;
	}

	for (; ; )
	{
		ToConsole(c_dropFile());
		cout("\n");
	}
}
