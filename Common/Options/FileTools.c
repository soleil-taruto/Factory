#include "FileTools.h"

#define DFDP_BUFFER_SIZE (1024 * 1024 * 1)

void DeleteFileDataPart(char *file, uint64 beginPos, uint64 delSize)
{
	uint64 size;
	FILE *rfp;
	FILE *wfp;
	void *buffer;
	uint64 remain;

	errorCase(m_isEmpty(file));

	size = getFileSize(file);

	errorCase(size < beginPos);
	errorCase(size - beginPos < delSize);

	if (delSize == 0)
		return;

	rfp = fileOpen(file, "rb");
	wfp = fileOpen(file, "r+b");

	fileSeek(rfp, SEEK_SET, beginPos + delSize);
	fileSeek(wfp, SEEK_SET, beginPos);

	buffer = memAlloc(DFDP_BUFFER_SIZE);

	for (remain = size - (beginPos + delSize); remain; )
	{
		uint rwSize = (uint)m_min(remain, (uint64)DFDP_BUFFER_SIZE);
		autoBlock_t gab;

		fileRead(rfp, gndBlockVar(buffer, rwSize, gab));
		fileWrite(wfp, gndBlockVar(buffer, rwSize, gab));

		remain -= rwSize;
	}
	memFree(buffer);

	fileClose(rfp);
	fileClose(wfp);

	setFileSize(file, size - delSize);
}
