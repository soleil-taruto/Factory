#include "C:\Factory\Common\all.h"
#include "C:\Factory\DevTools\libs\RandData.h"

// ---- copyBlock ----

static void DoTest_copyBlock(char *beforeData, uint destPos, uint srcPos, uint size, char *afterData)
{
	char *line = strx(beforeData);

	cout("line_1: %s\n", line);

	copyBlock(line + destPos, line + srcPos, size);

	cout("line_2: %s\n", line);
	cout("answer: %s\n", afterData);

	errorCase(strcmp(line, afterData));

	memFree(line);
}
static void Test_copyBlock(void)
{
	DoTest_copyBlock("AAABBBCCC", 0, 3, 6, "BBBCCCCCC");
	DoTest_copyBlock("AAABBBCCC", 3, 0, 6, "AAAAAABBB");
	DoTest_copyBlock("aaaaaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbbbbbcccccccccccccccccccc", 0, 20, 40, "bbbbbbbbbbbbbbbbbbbbcccccccccccccccccccccccccccccccccccccccc");
	DoTest_copyBlock("aaaaaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbbbbbcccccccccccccccccccc", 20, 0, 40, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbbbbb");

	cout("OK\n");
}

// ---- trimSequ ----

static void TestFunc_trimSequ(char *str)
{
	char *tmp = strx(str);

	tmp = replaceLineLoop(tmp, "  ", " ", 0, 20);
	strcpy(str, tmp);
	memFree(tmp);
}
static void DoTest_trimSequ(char *str, char *ext)
{
	uint strsz = strlen(str) + 1;
	uint extsz = strlen(ext) + 1;
	uint size;
	char *buff1;
	char *buff2;

//	cout("<0.1 [%s]\n", str);
//	cout("<0.2 [%s]\n", ext);

	size = strsz + extsz;

	buff1 = memAlloc(size);
	buff2 = memAlloc(size);

	memcpy(buff1,         str, strsz);
	memcpy(buff1 + strsz, ext, extsz);

	TestFunc_trimSequ(str);

	memcpy(buff2,         str, strsz);
	memcpy(buff2 + strsz, ext, extsz);

	trimSequ(buff1, ' ');

//	cout("1.1> [%s]\n", buff1);
//	cout("2.1> [%s]\n", buff2);
//	cout("1.2> [%s]\n", buff1 + strsz);
//	cout("2.2> [%s]\n", buff2 + strsz);

	errorCase(strcmp(buff1, buff2));
	errorCase(strcmp(buff1 + strsz, buff2 + strsz));

	memFree(buff1);
	memFree(buff2);
}
static void Test_trimSequ_LU(int low, int upper)
{
	uint c;

	cout("%02x - %02x\n", low, upper);

	MRLChrLow = low;
	MRLChrUpper = upper;

	for (c = 0; c < 1000; c++)
	{
		char *str = MakeRandLineRange(0, 70);
		char *ext = MakeRandLineRange(0, 70);

		DoTest_trimSequ(str, ext);

		memFree(str);
		memFree(ext);
	}
	cout("done\n");
}
static void Test_trimSequ(void)
{
	Test_trimSequ_LU(0x20, 0x20);
	Test_trimSequ_LU(0x20, 0x21);
	Test_trimSequ_LU(0x20, 0x7e);
	Test_trimSequ_LU(0x21, 0x7e);

	cout("OK\n");
}

// ----

int main(int argc, char **argv)
{
	mt19937_init32((uint)time(NULL));

	Test_copyBlock();
	Test_trimSequ();
}
