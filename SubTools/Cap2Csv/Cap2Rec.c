#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\csvStream.h"

static uint64 FirstRecPos = 0x48;
static uint64 RecSpan = 15;

static void Cap2Rec(char *rFile, char *wFile)
{
	FILE *rfp;
	FILE *wfp;

	errorCase(m_isEmpty(rFile));
	errorCase(m_isEmpty(wFile));

	cout("< %s\n", rFile);
	cout("> %s\n", wFile);
	cout("FirstRecPos: %I64u\n", FirstRecPos);
	cout("RecSpan: %I64u\n", RecSpan);

	rfp = fileOpen(rFile, "rb");
	wfp = fileOpen(wFile, "wb");

	fileSeek(rfp, SEEK_SET, FirstRecPos);

	for (; ; )
	{
		uint64 frame;
		uint size;
		uint subSize;
		autoBlock_t *data;

		frame = readValue64(rfp);
		size = readValue(rfp);
		subSize = readValue(rfp);

		if (feof(rfp))
		{
			LOGPOS();
			break;
		}
		if (size != subSize)
		{
			LOGPOS();
			break;
		}
		data = readBinaryBlock(rfp, size);

		if (getSize(data) < size)
		{
			LOGPOS();
			releaseAutoBlock(data);
			break;
		}
		fileSeek(rfp, SEEK_CUR, RecSpan);

		writeCSVCell_x(wfp, xcout("%I64u", frame));
		writeChar(wfp, ',');
		writeCSVCell_x(wfp, xcout("%u", size));
		writeChar(wfp, ',');
		writeCSVCell_x(wfp, makeHexLine(data));
		writeChar(wfp, '\n');

		releaseAutoBlock(data);
	}
	fileClose(rfp);
	fileClose(wfp);
}
int main(int argc, char **argv)
{
	char *rFile;
	char *wFile;

readArgs:
	if (argIs("/F"))
	{
		FirstRecPos = toValue64(nextArg());
		goto readArgs;
	}
	if (argIs("/S"))
	{
		RecSpan = toValue64(nextArg());
		goto readArgs;
	}

	rFile = makeFullPath(nextArg());
	wFile = makeFullPath(nextArg());

	errorCase(hasArgs(1)); // 不明なオプション
	errorCase(!mbs_stricmp(rFile, wFile)); // ? rFile == wFile

	Cap2Rec(rFile, wFile);

	memFree(rFile);
	memFree(wFile);
}
