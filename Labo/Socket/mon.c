#include "C:\Factory\Common\all.h"

static uint GetPtnCount(char *text, char *ptn)
{
	char *p = text;
	uint count = 0;

	for (; ; )
	{
		p = mbs_strstr(p, ptn);

		if (!p)
			break;

		p += strlen(ptn);
		count++;
	}
	return count;
}
int main(int argc, char **argv)
{
	char *outFile = makeTempFile(NULL);
	uint loopcnt;
	uint establishedNumPeak = 0;
	uint listeningNumPeak = 0;
	uint timeWaitNumPeak = 0;
	uint totalNumPeak = 0;

	cmdTitle("mon");

	hasArgs(0); // for ReadSysArgs()

	for (loopcnt = 0; ; loopcnt++)
	{
		char *outText;
		uint establishedNum;
		uint listeningNum;
		uint timeWaitNum;
		uint totalNum;

		execute_x(xcout("netstat -na > \"%s\"", outFile));

		outText = readText(outFile);

		establishedNum = GetPtnCount(outText, "ESTABLISHED");
		listeningNum   = GetPtnCount(outText, "LISTENING");
		timeWaitNum    = GetPtnCount(outText, "TIME_WAIT");

		memFree(outText);

		totalNum = establishedNum + listeningNum + timeWaitNum;

		cout("ESTABLISHED:%5u, LISTENING:%5u, TIME_WAIT:%5u, TOTAL:%5u\n", establishedNum, listeningNum, timeWaitNum, totalNum);

		m_maxim(establishedNumPeak, establishedNum);
		m_maxim(listeningNumPeak, listeningNum);
		m_maxim(timeWaitNumPeak, timeWaitNum);
		m_maxim(totalNumPeak, totalNum);

		if (loopcnt % 30 == 0)
		{
			cout("-------------------------------------------------------------------------------\n");
			cout("ESTABLISHED:%5u, LISTENING:%5u, TIME_WAIT:%5u, TOTAL:%5u *PEAK\n", establishedNumPeak, listeningNumPeak, timeWaitNumPeak, totalNumPeak);
			cout("-------------------------------------------------------------------------------\n");
			cout("%s\n", c_makeJStamp(NULL, 0));
		}

		// key wait
		{
			uint sec;

			for (sec = 0; sec < 5; sec++)
			{
				while (hasKey())
				{
					switch (getKey())
					{
					case '1':
						cout("*RESET PEAK\n");

						establishedNumPeak = 0;
						listeningNumPeak = 0;
						timeWaitNumPeak = 0;
						totalNumPeak = 0;

						break;

					case 0x1b:
						goto endLoop;

					default:
						cout("?\n");
						break;
					}
				}
				sleep(1000);
			}
		}
	}
endLoop:
	removeFile(outFile);
	memFree(outFile);
}
