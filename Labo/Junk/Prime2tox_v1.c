/*
	Prime2tox.exe [MAX_VAL]?

		MAX_VAL: 11 - (2^64-2)
*/

#include "C:\Factory\Common\all.h"

static FILE *SosuFp;
static uint64 SosuCnt;

static int IsSosu(uint64 value)
{
	uint64 count;

	fileSeek(SosuFp, SEEK_SET, 0);

	for (count = 0; count < SosuCnt; count++)
	{
		uint64 sosu = readValue64(SosuFp);

		if (0xffffffff < sosu) // 2bs
			break;

		if (value < sosu * sosu)
			break;

		if (value % sosu == 0)
			return 0;
	}
	return 1;
}
static void AddSosu(uint64 value)
{
	fileSeek(SosuFp, SEEK_END, 0);
	writeValue64(SosuFp, value);
	SosuCnt++;
}
static void DispSosu(void)
{
	uint64 count;

	cout("2\n");
	cout("3\n");
	cout("5\n");
	cout("7\n");

	fileSeek(SosuFp, SEEK_SET, 0);

	for (count = 0; count < SosuCnt; count++)
	{
		cout("%I64u\n", readValue64(SosuFp));
	}
}
int main(int argc, char **argv)
{
	char *sosuFile = makeTempFile("sosu");
	uint64 value;
	uint64 max = toValue64(nextArg());

	errorCase(max < 11);
	errorCase(max == UINT64MAX);

	SosuFp = fileOpen(sosuFile, "a+b");
	SosuCnt = 0;

	for (value = 11; value <= max; value += 2)
	{
		if ((value & 0x3ffe) == 0)
			cmdTitle_x(xcout("Prime2tox - %I64u", value));

		if (
			value % 3 != 0 &&
			value % 5 != 0 &&
			value % 7 != 0 &&
			IsSosu(value)
			)
			AddSosu(value);
	}
	cmdTitle("Prime2tox - Completed");
	DispSosu();

	fileClose(SosuFp);
	removeFile(sosuFile);
	memFree(sosuFile);
}
