#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Collabo.h"
#include "C:\Factory\SubTools\libs\wav.h"

#define JIHO_DELAY_SEC 5
#define MUON_SEC 1
//#define MUON_SEC 15

static char *GetToolkitExeFile(void)
{
	static char *file;

	if (!file)
//		file = GetCollaboFile("C:\\app\\Kit\\Toolkit\\Toolkit.exe"); // îpé~ @ 2023.4.13
		file = GetCollaboFile("C:\\Factory\\tmp\\Kit_Dummy\\Toolkit\\Toolkit.exe");

	return file;
}
static void InsertTopMuon(char *rFile, char *wFile)
{
	autoList_t *wavData = readWAVFile(rFile);
	autoList_t *dest = newList();
	uint count;
	uint wavTop;

	wavTop = getElement(wavData, 0);

	for (count = 0; count < lastWAV_Hz * MUON_SEC; count++)
		addElement(dest, wavTop);

	addElements(dest, wavData);

	writeWAVFile(wFile, dest, lastWAV_Hz);

	releaseAutoList(wavData);
	releaseAutoList(dest);
}

static char *JihoWavFile;

static void PlayJiho(void)
{
	char *wkJWFile = makeTempPath("wav");

	InsertTopMuon(JihoWavFile, wkJWFile);

	LOGPOS();
	coExecute_x(xcout("START \"\" /B /WAIT \"%s\" /PLAY-WAV \"%s\"", GetToolkitExeFile(), wkJWFile));
	LOGPOS();

	removeFile_x(wkJWFile);
}
int main(int argc, char **argv)
{
	JihoWavFile = nextArg();

	errorCase(!existFile(JihoWavFile));
	errorCase(_stricmp(getExt(JihoWavFile), "wav"));

	if (argIs("/P"))
	{
		PlayJiho();
		return;
	}

	LOGPOS();

	for (; ; )
	{
		uint rem = (3600 - (uint)((time(NULL) + MUON_SEC + JIHO_DELAY_SEC) % 3600)) % 3600;
		uint millis;

		if (rem == 0)
		{
			PlayJiho();
		}
		else if (rem == 1)
		{
			millis = 100;
		}
		else if (rem == 2)
		{
			millis = 300;
		}
		else if (rem == 3)
		{
			millis = 500;
		}
		else if (rem <= 10)
		{
			millis = 1000;
		}
		else
		{
			millis = 5000;
		}

		LOGPOS();
		cout("rem, millis: %u, %u\n", rem, millis);

		{
			int key = waitKey(millis);

			if (key == 0x1b)
				break;

			if (key == 0x0d)
			{
				cout("################\n");
				cout("## ÉeÉXÉgéûïÒ ##\n");
				cout("################\n");

				PlayJiho();
			}
		}
	}
	LOGPOS();
}
