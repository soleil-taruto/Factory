#include "C:\Factory\Common\all.h"

#define NUMB_SPAN 3936092160 // == PBIT_P_NUM @ Prime2.c

static int CountMode;
static uint64 MinValue = 2;
static uint64 MaxValue = 100000000000; // 0.1-chou

static void MakeBatch(uint64 minval, uint64 maxval, uint bat_no)
{
	writeOneLine_xx(
		xcout("%04u.bat_undone", bat_no),
		xcout("Prime64.exe %s %I64u %I64u out\\%04u_%I64u-%I64u.txt", CountMode ? "/C" : "/R", minval, maxval, bat_no, minval, maxval)
		);
}
static void MkBatUndone(void)
{
	char *localDir;
	char *dir;
	uint64 count;
	uint bat_no;

	errorCase(MaxValue < MinValue);
	errorCase(NUMB_SPAN * 9998ui64 < MaxValue - MinValue); // スパンが長すぎる。

	if (CountMode)
		localDir = xcout("PrimeCount_%I64u-%I64u", MinValue, MaxValue);
	else
		localDir = xcout("Prime_%I64u-%I64u", MinValue, MaxValue);

	dir = getOutFile(localDir);
	createDir(dir);
	addCwd(dir);

	count = MinValue;

	for (bat_no = 1; ; bat_no++)
	{
		uint64 end;

		if (count < (UINT64MAX / NUMB_SPAN) * NUMB_SPAN)
			end = (count / NUMB_SPAN + 1) * NUMB_SPAN;
		else
			end = UINT64MAX;

		m_minim(end, MaxValue);

		MakeBatch(count, end, bat_no);

		if (end == MaxValue)
			break;

		count = end;
	}

	// その他のファイル
	{
		copyFile("C:\\Factory\\Labo\\Tools\\BatToriai.exe", "BatToriai.exe");
		copyFile("C:\\Factory\\Program\\Prime64\\Prime64.exe", "Prime64.exe");

		coExecute("C:\\Factory\\SubTools\\EmbedConfig.exe --factory-dir-disabled BatToriai.exe");
		coExecute("C:\\Factory\\SubTools\\EmbedConfig.exe --factory-dir-disabled Prime64.exe");

		coExecute("BatToriai.exe . /L");
		coExecute("Prime64.exe /P 1");

		writeOneLine("_Go_1.bat", "BatToriai.exe .");
		writeOneLine("_Go.bat", "FOR /L %%C IN (1,1,%NUMBER_OF_PROCESSORS%) DO START CMD /C _Go_1.bat");
		writeOneLine("_Stop.bat", "BatToriai.exe . /S");

		if (CountMode)
		{
			copyFile("C:\\Factory\\Labo\\Tools\\Prime64\\sub\\MakeSum.exe", "MakeSum.exe");
			coExecute("C:\\Factory\\SubTools\\EmbedConfig.exe --factory-dir-disabled MakeSum.exe");
			writeOneLine("_MakeSum.bat", "MakeSum.exe out out\\_Sum.txt\nSTART out\\_Sum.txt");
		}
		else
		{
			copyFile("C:\\Factory\\Labo\\Tools\\Prime64\\sub\\MakeJoined.exe", "MakeJoined.exe");
			coExecute("C:\\Factory\\SubTools\\EmbedConfig.exe --factory-dir-disabled MakeJoined.exe");
			writeOneLine("_MakeJoined.bat", "MakeJoined.exe out out\\_Joined.txt\nSTART out"); // でか過ぎてエディタで開けないのでフォルダを開く。
		}
		createDir("out");
		createDir("tmp");
	}

	unaddCwd();
	memFree(localDir);
	memFree(dir);
	openOutDir();
}
int main(int argc, char **argv)
{
readArgs:
	if (argIs("/C"))
	{
		CountMode = 1;
		goto readArgs;
	}
	if (argIs("/R"))
	{
		MinValue = toValue64(nextArg());
		MaxValue = toValue64(nextArg());
		goto readArgs;
	}
	MkBatUndone();
}
