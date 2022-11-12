#include "C:\Factory\Common\all.h"
#include "C:\Factory\SubTools\libs\wavFile.h"

int main(int argc, char **argv)
{
	if (argIs("/W2C"))
	{
		char *rWavFile;
		char *wCsvFile;
		char *wHzFile;

		rWavFile = nextArg();
		wCsvFile = nextArg();
		wHzFile  = nextArg();

		errorCase(m_isEmpty(rWavFile));
		errorCase(m_isEmpty(wCsvFile));
		errorCase(m_isEmpty(wHzFile));

		LOGPOS();
		readWAVFileToCSVFile(rWavFile, wCsvFile);
		LOGPOS();
		writeOneLineNoRet_b_cx(wHzFile, xcout("%u", lastWAV_Hz));
		LOGPOS();
		return;
	}
	if (argIs("/C2W"))
	{
		char *rCsvFile;
		char *rHzFile;
		char *wWavFile;
		uint hz;

		rCsvFile = nextArg();
		rHzFile  = nextArg();
		wWavFile = nextArg();

		errorCase(m_isEmpty(rCsvFile));
		errorCase(m_isEmpty(rHzFile));
		errorCase(m_isEmpty(wWavFile));

		LOGPOS();
		hz = toValue_x(readFirstLine(rHzFile));
		LOGPOS();

		errorCase(!m_isRange(hz, 1, 1000000)); // ”ÍˆÍ‚Í“K“–

		LOGPOS();
		writeWAVFileFromCSVFile(rCsvFile, wWavFile, hz);
		LOGPOS();
		return;
	}
}
