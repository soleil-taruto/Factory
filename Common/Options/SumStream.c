#include "SumStream.h"

#define SUMSIZE 16

static md5_t *Context;
static uchar Sum[SUMSIZE];

static void InitSum(void)
{
	Context = md5_create();
}
static void UpdateSum(uchar *block, uint size)
{
	autoBlock_t gab = gndBlock(block, size);

	md5_update(Context, &gab);
}
static void MakeSum(void)
{
	autoBlock_t *bSum = md5_makeHash(Context);

	memcpy(Sum, directGetBuffer(bSum), SUMSIZE);
	releaseAutoBlock(bSum);

	md5_release(Context);
}

static void (*UserListener)(uchar *, uint);

static void StreamListener(uchar *block, uint size)
{
	UserListener(block, size);
	UpdateSum(block, size); // streamReader() ÇÃÇΩÇﬂÇ…å„ÇÎÇ…íuÇ≠ÅB
}

static void ToSumStream(void *src, int (*srcToStream)(void *, void (*)(uchar *, uint)), void (*streamWriter)(uchar *, uint))
{
	InitSum();

	UserListener = streamWriter;
	srcToStream(src, StreamListener);

	MakeSum();
	streamWriter(Sum, SUMSIZE);
}
void VTreeToSumStream(VTree_t *vt, void (*streamWriter)(uchar *, uint))
{
	ToSumStream(vt, (int (*)(void *, void (*)(uchar *, uint)))VTreeToStream, streamWriter);
}
void DirToSumStream(char *dir, void (*streamWriter)(uchar *, uint))
{
	ToSumStream(dir, (int (*)(void *, void (*)(uchar *, uint)))DirToStream, streamWriter);
}
int SumStreamToDir(char *dir, void (*streamReader)(uchar *, uint))
{
	uchar rdSum[SUMSIZE];

	InitSum();

	UserListener = streamReader;
	StreamToDir(dir, StreamListener);

	MakeSum();
	streamReader(rdSum, SUMSIZE);

	return !memcmp(rdSum, Sum, SUMSIZE);
}

#define R_BUFF_SIZE (1024 * 1024 * 16)

int CheckSumStream(void (*streamReader)(uchar *, uint), uint64 streamLength)
{
	uchar rdSum[SUMSIZE];
	uchar *buffer;
	uint64 count;

	if (streamLength < SUMSIZE)
		return 0;

	InitSum();

	buffer = (uchar *)memAlloc(R_BUFF_SIZE);
	count = streamLength - SUMSIZE;

	while (R_BUFF_SIZE < count)
	{
		streamReader(buffer, R_BUFF_SIZE);
		UpdateSum(buffer, R_BUFF_SIZE);
		count -= R_BUFF_SIZE;
	}
	streamReader(buffer, count);
	UpdateSum(buffer, count);
	memFree(buffer);

	MakeSum();
	streamReader(rdSum, SUMSIZE);

	return !memcmp(rdSum, Sum, SUMSIZE);
}
