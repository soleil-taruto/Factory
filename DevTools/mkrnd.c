/*
	mkrnd.exe [/S SEED] ...

	mkrnd.exe ... /B OUTPUT-FILE SIZE

	mkrnd.exe ... [/CB | /CO | /CD | /CH | /CS | /CC CHAR-SET | /CR CHAR-MIN CHAR-MAX] /T OUTPUT-FILE ROW-NUM (COL-NUM | COL-MIN COL-MAX)

	----
	実行例

	mkrnd.exe /ch /t 1.txt 32 64

		mkcrと同じ書式の 1.txt を出力する。この場合 CSPRNG ではないことに注意！
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\DevTools\libs\RandData.h"

int main(int argc, char **argv)
{
	mt19937_init();

readArgs:
	if (argIs("/S")) // Seed
	{
		mt19937_init32(toValue(nextArg()));
		goto readArgs;
	}

	if (argIs("/CB")) // Char Bina-decimal
	{
		MRLChrSet = binadecimal;
		MRLGetChar = MRLGetCharSet;
		goto readArgs;
	}
	if (argIs("/CO")) // Char Octo-decimal
	{
		MRLChrSet = octodecimal;
		MRLGetChar = MRLGetCharSet;
		goto readArgs;
	}
	if (argIs("/CD")) // Char Decimal
	{
#if 1
		MRLChrSet = decimal;
		MRLGetChar = MRLGetCharSet;
#else // Same?
		MRLChrLow = '0';
		MRLChrUpper = '9';
		MRLGetChar = MRLGetCharRange;
#endif
		goto readArgs;
	}
	if (argIs("/CH")) // Char Hexa-decimal
	{
		MRLChrSet = hexadecimal;
		MRLGetChar = MRLGetCharSet;
		goto readArgs;
	}
	if (argIs("/CS")) // Char enable-Space
	{
		MRLChrLow = 0x20;
		MRLChrUpper = 0x7e;
		MRLGetChar = MRLGetCharRange;
		goto readArgs;
	}

	if (argIs("/CC")) // Char Char-set
	{
		char *cs = nextArg();

		errorCase(!*cs); // ? == ""

		MRLChrSet = cs;
		MRLGetChar = MRLGetCharSet;
		goto readArgs;
	}
	if (argIs("/CR")) // Char Range
	{
		uint r1;
		uint r2;

		r1 = toValue(nextArg());
		r2 = toValue(nextArg());

		errorCase(r1 < 1);
		errorCase(r2 < r1);
		errorCase(255 < r2);

		MRLChrLow = r1;
		MRLChrUpper = r2;
		MRLGetChar = MRLGetCharRange;
		goto readArgs;
	}

	if (argIs("/B")) // Binary mode
	{
		char *file;
		uint64 size;

		file = nextArg();
		size = toValue64(nextArg());

		MakeRandBinaryFile(file, size);

		cout("バイナリで作成しました。\n");
		return;
	}
	if (argIs("/T")) // Text mode
	{
		char *file;
		uint64 row;
		uint column;

		file = nextArg();
		row = toValue64(nextArg());
		column = toValue(nextArg());

		if (hasArgs(1))
		{
			MakeRandTextFileRange(file, row, column, toValue(nextArg())); // max < min のとき mt19937_range() でエラーになってくれる。
		}
		else
		{
			MakeRandTextFile(file, row, column);
		}
		cout("テキストで作成しました。\n");
		return;
	}
}
