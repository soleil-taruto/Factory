#include "C:\Factory\Common\all.h"

#define SOSUMAP_SIZE (2 * 3 * 5 * 7 * 11 * 13 * 17 * 19 * 23)

static uint SosuList[] =
{
	2,
	3,
	5,
	7,
	11,
	13,
	17,
	19,
	23,
};

static uint64 CountMax;
static uchar *SosuMap;
static char *SosuFile;
static FILE *SosuFp;

static uint Root64(uint64 value)
{
	uint root = 0;
	uint bit;

	for (bit = 1u << 31; bit; bit >>= 1)
	{
		uint r = root | bit;

		if ((uint64)r * r <= value)
		{
			root = r;
		}
	}
	return root;
}
static void MakeSosuMap(void)
{
	uint index;
	uint ndx;

	SosuMap = (uchar *)memAlloc(SOSUMAP_SIZE);
	SosuMap[0] = 0;
	memset(SosuMap + 1, 1, SOSUMAP_SIZE - 1);

	for (index = 0; index < lengthof(SosuList); index++)
	{
		for (ndx = SosuList[index]; ndx < SOSUMAP_SIZE; ndx += SosuList[index])
		{
			SosuMap[ndx] = 0;
		}
	}
}
static void WriteSosuMap(void)
{
	uint64 count = CountMax + 1;
	autoBlock_t gab;

	while (SOSUMAP_SIZE < count)
	{
		fileWrite(SosuFp, gndBlockVar(SosuMap, SOSUMAP_SIZE, gab));
		count -= SOSUMAP_SIZE;
	}
	fileWrite(SosuFp, gndBlockVar(SosuMap, (uint)count, gab));
}
static void DispSosu(void)
{
	uint index;
	uint64 count;

	for (index = 0; index < lengthof(SosuList); index++)
	{
		cout("%u\n", SosuList[index]);
	}
	fileSeek(SosuFp, SEEK_SET, 29);

	for (count = 29; count <= CountMax; count++)
	{
		if (readChar(SosuFp))
		{
			cout("%I64u\n", count);
		}
	}
}
int main(int argc, char **argv)
{
	uint count;
	uint rootCntMax;
	uint64 wcnt;

	CountMax = toValue64(nextArg());

	errorCase(CountMax < 23);
	errorCase(CountMax == UINT64MAX);

	MakeSosuMap();

	SosuFile = makeTempFile("sosu");
	SosuFp = fileOpen(SosuFile, "r+b");

	WriteSosuMap();

	rootCntMax = Root64(CountMax);

	for (count = 29; count <= rootCntMax; count++)
	{
		if (count < 100 || count % 100 == 0)
			cmdTitle_x(xcout("Prime2tox - %I64u ‚È‚¤...", (uint64)count * count));

		fileSeek(SosuFp, SEEK_SET, (uint64)count);

		if (readChar(SosuFp))
		{
			for (wcnt = (uint64)count * 3; wcnt <= CountMax; wcnt += (uint64)count * 2)
			{
				fileSeek(SosuFp, SEEK_SET, wcnt);
				writeChar(SosuFp, 0);
			}
		}
	}
	cmdTitle("Prime2tox - ŒvŽZ‚¨‚í‚è");
	DispSosu();

	memFree(SosuMap);
	fileClose(SosuFp);
	removeFile(SosuFile);
	memFree(SosuFile);
}
