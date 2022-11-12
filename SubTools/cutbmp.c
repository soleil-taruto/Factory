#include "C:\Factory\Common\all.h"
#include "libs\bmp.h"

static autoList_t *CutBmp(autoList_t *bmp, uint l, uint t, uint w, uint h)
{
	autoList_t *cBmp = newList();
	uint x;
	uint y;

	for (x = 0; x < w; x++)
	for (y = 0; y < h; y++)
	{
		autoList_t *row = (autoList_t *)refElement(bmp, t + y);
		uint dot = 0;

		if (row)
			dot = refElement(row, l + x);

		putElement(refList(cBmp, y), x, dot);
	}
	return cBmp;
}
static void CutBmpFile(char *rFile, uint l, uint t, uint w, uint h, char *wFile)
{
	autoList_t *bmp;
	autoList_t *cBmp;

	if (!wFile)
	{
		wFile = changeExt(rFile, "");
		wFile = addLine(wFile, "_cut.bmp");
	}

	cout("< %s\n", rFile);
	cout("# %u %u %u %u\n", l, t, w, h);
	cout("> %s\n", wFile);

	errorCase(!w);
	errorCase(!h);
	errorCase(UINTMAX - l < w);
	errorCase(UINTMAX - t < h);

	bmp = readBMPFile(rFile);
	cBmp = CutBmp(bmp, l, t, w, h);
	releaseDim_BR(bmp, 2, NULL);
	writeBMPFile_cx(wFile, cBmp);
}

int main(int argc, char **argv)
{
	if (hasArgs(6))
	{
		CutBmpFile(
			getArg(0),
			toValue(getArg(1)),
			toValue(getArg(2)),
			toValue(getArg(3)),
			toValue(getArg(4)),
			getArg(5)
			);

		return;
	}
	if (hasArgs(5))
	{
		CutBmpFile(
			getArg(0),
			toValue(getArg(1)),
			toValue(getArg(2)),
			toValue(getArg(3)),
			toValue(getArg(4)),
			NULL
			);

		return;
	}
	if (hasArgs(4))
	{
		uint l = toValue(getArg(0));
		uint t = toValue(getArg(1));
		uint w = toValue(getArg(2));
		uint h = toValue(getArg(3));

		for (; ; )
		{
			CutBmpFile(c_dropFile(), l, t, w, h, NULL);
			cout("\n");
		}
	}
}
