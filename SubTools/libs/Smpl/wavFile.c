/*
	wavFile.exe /W2C WAV-FILE CSV-FILE
	wavFile.exe /C2W CSV-FILE WAV-FILE
*/

#include "C:\Factory\Common\all.h"
#include "..\wavFile.h"

int main(int argc, char **argv)
{
	if (argIs("/W2C"))
	{
		char *rFile;
		char *wFile;

		rFile = nextArg();
		wFile = nextArg();

		readWAVFileToCSVFile(rFile, wFile);
		cout("lastWAV_Hz: %u\n", lastWAV_Hz);
		return;
	}
	if (argIs("/C2W"))
	{
		char *rFile;
		char *wFile;
		uint hz;

		rFile = nextArg();
		wFile = nextArg();
		hz = toValue(nextArg());

		writeWAVFileFromCSVFile(rFile, wFile, hz);
		return;
	}
}
