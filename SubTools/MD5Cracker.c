/*
	MD5Cracker.exe [/A | /9 | /C 開始文字と終了文字] [/F md5リストファイル | md5]

	- - -
	実行例

	MD5Cracker 7f138a09169b250e9dcb378140907378
	MD5Cracker /9 25d55ad283aa400af464c76d713c07ad
	MD5Cracker /C 0z 3b2f8d5774b260b0986140894491ae51
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\OpenSource\md5\md5.h"

#define HASH_SIZE 16

static int BChrMin = 0x00;
static int BChrMax = 0xff;

static autoList_t *HashTable;
static uint RemHashCount;

static void InitHashTable(autoList_t *lines)
{
	char *line;
	uint index;

	HashTable = newList();

	for (index = 0; index < 0x10000; index++)
		addElement(HashTable, (uint)newList());

	foreach (lines, line, index)
	{
		uchar *hash;
		uint segment;
		autoList_t *hashes;

		cout("INPUT_HASH: %s\n", line);

		errorCase(!lineExp("<32,09AFaf>", line));

		hash = (uchar *)unbindBlock(ab_fromHexLine(line));
		segment = (uint)*(uint16 *)hash;
		hashes = (autoList_t *)getElement(HashTable, segment);
		addElement(hashes, (uint)hash);
	}

	RemHashCount = getCount(lines);
	errorCase(!RemHashCount);
}
static void Found(uchar *msg, uint msgLen, uchar msgHash[HASH_SIZE])
{
	char *h1;
	char *h2;
	char *pmsg;
	autoBlock_t gab;

	h1 = makeHexLine(gndBlockVar(msgHash, HASH_SIZE, gab));
	h2 = makeHexLine(gndBlockVar(msg, msgLen, gab));
	pmsg = toPrintLine(gndBlockVar(msg, msgLen, gab), 0);

	cout("%s = %s [%s]\n", h1, h2, pmsg);

	memFree(h1);
	memFree(h2);
	memFree(pmsg);

	if (!--RemHashCount)
		termination(0);
}
static void Check(uchar *msg, uint msgLen, uchar msgHash[HASH_SIZE])
{
	uint segment = (uint)*(uint16 *)msgHash;
	autoList_t *hashes;
	uchar *hash;
	uint index;

	hashes = (autoList_t *)getElement(HashTable, segment);

	foreach (hashes, hash, index)
	{
		if (!memcmp(hash, msgHash, HASH_SIZE))
		{
			Found(msg, msgLen, hash);

			desertElement(hashes, index);
			break;
		}
	}
}
#if 1
static void Search(uint msgLen)
{
	md5_CTX ctxs[100];
	uchar msg[100];
	uint index = 0;
	int ahead = 1;

	md5_Init(ctxs);

	for (; ; )
	{
		if (ahead)
		{
			if (index < msgLen)
			{
				msg[index] = BChrMin;
				ctxs[index + 1] = ctxs[index];
				md5_Update(ctxs + index + 1, msg + index, 1);
			}
			else
			{
				md5_Final(ctxs + index);

				Check(msg, msgLen, ctxs[index].digest);

				ahead = 0;
			}
		}
		else
		{
			if (msg[index] < BChrMax)
			{
				msg[index]++;
				ctxs[index + 1] = ctxs[index];
				md5_Update(ctxs + index + 1, msg + index, 1);

				ahead = 1;
			}
			else
			{
				// noop
			}
		}

		if (ahead)
		{
			index++;
		}
		else
		{
			if (!index)
				break;

			index--;
		}
	}
}
static void MD5Crack_Main(void)
{
	uint msgLen;

	for (msgLen = 0; ; msgLen++)
	{
		cout("msgLen: %u\n", msgLen);

		Search(msgLen);
	}
}
#else // old same
static void Search(uchar *msg, uint msgLen, uint index, md5_CTX *baseCtx)
{
	if (index < msgLen)
	{
		uint bChr;

		for (bChr = BChrMin; bChr <= BChrMax; bChr++)
		{
			md5_CTX ctx = *baseCtx;

			msg[index] = bChr;
			md5_Update(&ctx, msg + index, 1);

			Search(msg, msgLen, index + 1, &ctx);
		}
	}
	else
	{
		md5_CTX ctx = *baseCtx;

		md5_Final(&ctx);

		Check(msg, msgLen, ctx.digest);
	}
}
static void MD5Crack_Main(void)
{
	uchar msg[100];
	uint msgLen;

	for (msgLen = 0; ; msgLen++)
	{
		md5_CTX ctx;

		md5_Init(&ctx);

		cout("msgLen: %u\n", msgLen);

		Search(msg, msgLen, 0, &ctx);
	}
}
#endif
static void MD5Crack_List(autoList_t *lines)
{
	distinct2(lines, (sint (*)(uint, uint))_stricmp, (void (*)(uint))memFree); // MD5Crack()の場合は1つなので問題無いはず。

	InitHashTable(lines);

	cout("BChrMin: %02x\n", BChrMin);
	cout("BChrMax: %02x\n", BChrMax);

	MD5Crack_Main();
}
static void MD5Crack(char *line)
{
	autoList_t gal;
	uint box;

	MD5Crack_List(gndOneElementVar((uint)line, box, gal));
}
int main(int argc, char **argv)
{
readArgs:
	if (argIs("/A"))
	{
		BChrMin = 0x20;
		BChrMax = 0x7e;
		goto readArgs;
	}
	if (argIs("/9"))
	{
		BChrMin = '0';
		BChrMax = '9';
		goto readArgs;
	}
	if (argIs("/C"))
	{
		char *chrs = nextArg();

		errorCase(strlen(chrs) != 2);
		errorCase(chrs[1] <= chrs[0]);

		BChrMin = chrs[0];
		BChrMax = chrs[1];
		goto readArgs;
	}

	if (argIs("/F"))
	{
		MD5Crack_List(readLines(nextArg())); // g
		return;
	}

	if (hasArgs(1))
	{
		MD5Crack(nextArg());
		return;
	}
	else
	{
		MD5Crack_List(readLines(c_dropFile())); // g
		return;
	}
}
