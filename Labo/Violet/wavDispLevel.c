/*
	wavDispLevel.exe [WAV-FILE]
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\SubTools\libs\wav.h"

#define HZ 44100
#define HZ_PER_DECISEC (HZ / 10)
#define MUON_LEVEL 32768

static autoList_t *WavData;

static void DispLevel(void)
{
	uint deciSec;
	uint index;
	uint ndx;

	for (deciSec = 0; ; deciSec++)
	{
		uint hi = 0;
		uint lw = IMAX;
		uint lv;

		for (index = 0; index < HZ_PER_DECISEC; index++)
		{
			ndx = deciSec * HZ_PER_DECISEC + index;

			if (getCount(WavData) <= ndx)
				break;

			lv = getElement(WavData, ndx) >> 16;

			m_maxim(hi, lv);
			m_minim(lw, lv);

			lv = getElement(WavData, ndx) & 0xffff;

			m_maxim(hi, lv);
			m_minim(lw, lv);
		}
		if (!index)
			break;

		cout("%02u:%02u.%u %6d, %6d\n"
			,deciSec / 600
			,deciSec / 10 % 60
			,deciSec % 10
			,(sint)lw - MUON_LEVEL
			,(sint)hi - MUON_LEVEL
			);
	}
endLoop:;
}
static void DispLevel_F(char *file)
{
	WavData = readWAVFile(file);
	DispLevel();
	releaseAutoList(WavData);
	WavData = NULL;
}
int main(int argc, char **argv)
{
	if (hasArgs(1))
	{
		char *file = nextArg();

		DispLevel_F(file);
		return;
	}

	for (; ; )
	{
		char *file = c_dropFile();

		DispLevel_F(file);
		cout("\n");
	}
}
