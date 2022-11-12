#include "C:\Factory\Common\all.h"

static uint RecordSize = 16;

static void ReadRecord(FILE *fp, uchar *dest)
{
	errorCase(fread(dest, 1, RecordSize, fp) != RecordSize);
}
static BinMD5MergeCounter(char *file1, char *file2, char *outFile)
{
	uchar *buff1 = (uchar *)memAlloc(RecordSize);
	uchar *buff2 = (uchar *)memAlloc(RecordSize);
	FILE *fp1 = fileOpen(file1, "rb");
	FILE *fp2 = fileOpen(file2, "rb");
	uint64 size1;
	uint64 size2;
	uint64 count1;
	uint64 count2;
	uint64 rPos1 = 0;
	uint64 rPos2 = 0;
	uint64 countBoth = 0;
	uint64 countOnly1;
	uint64 countOnly2;

	size1 = getFileSizeFPSS(fp1);
	size2 = getFileSizeFPSS(fp2);

	errorCase(size1 % RecordSize != 0);
	errorCase(size2 % RecordSize != 0);

	count1 = size1 / RecordSize;
	count2 = size2 / RecordSize;

	goto readBoth;

	for (; ; )
	{
		int ret = memcmp(buff1, buff2, RecordSize);

		if (ret < 0)
		{
			if (count1 <= rPos1)
				break;

			ReadRecord(fp1, buff1);
			rPos1++;
		}
		else if (0 < ret)
		{
			if (count2 <= rPos2)
				break;

			ReadRecord(fp2, buff2);
			rPos2++;
		}
		else
		{
			countBoth++;

		readBoth:
			if (count1 <= rPos1 || count2 <= rPos2)
				break;

			ReadRecord(fp1, buff1);
			ReadRecord(fp2, buff2);
			rPos1++;
			rPos2++;
		}
	}
	countOnly1 = count1 - countBoth;
	countOnly2 = count2 - countBoth;

	writeOneLineNoRet_b_cx(outFile, xcout("%I64u,%I64u,%I64u", countOnly1, countBoth, countOnly2));

	memFree(buff1);
	memFree(buff2);
	fileClose(fp1);
	fileClose(fp2);
}
int main(int argc, char **argv)
{
	if (argIs("/SZ"))
	{
		RecordSize = toValue(nextArg());
	}

	errorCase(RecordSize < 1);

	BinMD5MergeCounter(getArg(0), getArg(1), getArg(2));
}
