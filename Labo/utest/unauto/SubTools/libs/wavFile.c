#include "C:\Factory\Common\all.h"
#include "C:\Factory\SubTools\libs\wavFile.h"

static void DoTest(char *wavFile)
{
	// ÉÅÉÇÉä
	{
		autoList_t *wavData;

		LOGPOS();
		wavData = readWAVFile(wavFile);
		LOGPOS();
		writeWAVFile("1.wav", wavData, lastWAV_Hz);
		LOGPOS();
		releaseAutoList(wavData);
	}

	// ÉtÉ@ÉCÉã
	{
		LOGPOS();
		readWAVFileToCSVFile(wavFile, "2.csv");
		LOGPOS();
		writeWAVFileFromCSVFile("2.csv", "2.wav", lastWAV_Hz);
		LOGPOS();
	}

	errorCase(!isSameFile("1.wav", "2.wav"));

	cout("OK\n");

	removeFile("1.wav");
	removeFile("2.csv");
	removeFile("2.wav");
}
int main(int argc, char **argv)
{
	for (; ; )
	{
		char *wavFile = c_dropFile();

		DoTest(wavFile);
		cout("\n");
	}
}
