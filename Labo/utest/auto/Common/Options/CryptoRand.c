#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\CryptoRand.h"

#define CRAND_B_EXE "C:\\Factory\\Labo\\Tools\\CryptoRand_B.exe"
#define TMPOUT_FILE "C:\\Factory\\tmp\\CryptoRand_utest_Test.tmp"

static double GetSameByteRate(uchar *a, uchar *b, uint size)
{
	uint sameCnt = 0;
	uint index;
	double rate;

	cout("GSBR: %p, %p, %u\n", a, b, size);

	for (index = 0; index < size; index++)
		if (a[index] == b[index])
			sameCnt++;

	rate = (double)sameCnt / size;
	cout("GSBR_ret: %.6f (%u / %u)\n", rate, sameCnt, size);
	return rate;
}
static void DoTest_01(void)
{
	// 初期状態に依存しないはず。
	// --> プロセスごとに乱数列は異なるはず。

#define TEST_NUM 100
#define DATA_SIZE    10000
#define S_DATA_SIZE "10000"

	static uchar tbl[TEST_NUM][DATA_SIZE];
	uint index;
	uint ndx;

	LOGPOS();

	for (index = 0; index < TEST_NUM; index++)
	{
		autoBlock_t *data;

		execute(CRAND_B_EXE " " S_DATA_SIZE " " TMPOUT_FILE);

		data = readBinary(TMPOUT_FILE);
		errorCase(getSize(data) != DATA_SIZE);

		memcpy(tbl[index], directGetBuffer(data), DATA_SIZE);

		releaseAutoBlock(data);
		removeFile(TMPOUT_FILE);
	}
	LOGPOS();
	for (index = 1; index < TEST_NUM; index++)
	for (ndx = 0; ndx < index; ndx++)
	{
		errorCase(!memcmp(tbl[ndx], tbl[index], DATA_SIZE));
	}
	LOGPOS();
	for (index = 1; index < TEST_NUM; index++)
	for (ndx = 0; ndx < index; ndx++)
	{
		errorCase(0.1 < GetSameByteRate(tbl[ndx], tbl[index], DATA_SIZE)); // 各バイトが一致する確率は 1/256  -->  1/10 超えはおかしい。
	}
	LOGPOS();

#undef TEST_NUM
#undef DATA_SIZE
#undef S_DATA_SIZE
}
static void DoTest_01a(void)
{
	// 各バイトが一致する確率は 1/256 に収束するはず。

#define DATA_SIZE    400000000
#define S_DATA_SIZE "400000000"
//#define DATA_SIZE    500000000
//#define S_DATA_SIZE "500000000"

	uint testCnt;

	LOGPOS();

	for (testCnt = 0; testCnt < 10; testCnt++)
	{
		uchar (*tbl)[DATA_SIZE] = (uchar (*)[DATA_SIZE])memAlloc(DATA_SIZE * 2);
		uint index;
		double rate;

		LOGPOS();

		for (index = 0; index < 2; index++)
		{
			autoBlock_t *data;

			execute(CRAND_B_EXE " " S_DATA_SIZE " " TMPOUT_FILE);

			data = readBinary(TMPOUT_FILE);
			errorCase(getSize(data) != DATA_SIZE);

			memcpy(tbl[index], directGetBuffer(data), DATA_SIZE);

			releaseAutoBlock(data);
			removeFile(TMPOUT_FILE);
		}
		LOGPOS();
		rate = GetSameByteRate(tbl[0], tbl[1], DATA_SIZE);

		// 1 / 256 == 0.00390625

		// --> 0.00390625 +- 0.0001 の範囲に収まるはず。多分。

		errorCase(!m_isRange(rate, 0.0038, 0.004));

		LOGPOS();

		memFree(tbl);
	}
	LOGPOS();

#undef DATA_SIZE
#undef S_DATA_SIZE
}
static void DoTest_02(void)
{
	// Test01a と同じ。

#define DATA_SIZE    400000000
#define S_DATA_SIZE "400000000"

	uint testCnt;

	LOGPOS();

	for (testCnt = 0; testCnt < 10; testCnt++)
	{
		uchar (*tbl)[DATA_SIZE] = (uchar (*)[DATA_SIZE])memAlloc(DATA_SIZE * 2);
		uint index;
		double rate;

		LOGPOS();

#if 1
		for (index = 0; index < 2; index++)
		{
			autoBlock_t *data = makeCryptoRandBlock(DATA_SIZE);

			memcpy(tbl[index], directGetBuffer(data), DATA_SIZE);

			releaseAutoBlock(data);
		}

#else // メモリ足らん。@ 2022.11.12
		{
			autoBlock_t *data = makeCryptoRandBlock(DATA_SIZE * 2);

			memcpy(tbl[0], directGetBuffer(data), DATA_SIZE * 2);

			releaseAutoBlock(data);
		}
#endif

		LOGPOS();
		rate = GetSameByteRate(tbl[0], tbl[1], DATA_SIZE);
		errorCase(!m_isRange(rate, 0.0038, 0.004));
		LOGPOS();

		memFree(tbl);
	}
	LOGPOS();

#undef DATA_SIZE
#undef S_DATA_SIZE
}
static void DoTest(void)
{
	DoTest_01();
	DoTest_01a();
	DoTest_02();
}
int main(int argc, char **argv)
{
	DoTest();
}
