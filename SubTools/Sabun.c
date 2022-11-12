#include "C:\Factory\Common\all.h"
#include "libs\Sabun\all.h"

static FILE *WFp;
static int VerboseMode;

static void CloseWFp(void)
{
	fileClose(WFp);
}
#define WL(format, ...) \
	(writeToken_x(WFp, xcout(format, __VA_ARGS__)))

static void DispSabun(Sabun_t *s)
{
	WL("<Sabun>\n");
	WL("<Pos>%u</Pos>\n", s->Pos);
	WL("<Length>%u</Length>\n", s->Length);

	if (s->Data)
	{
		uint index;

		WL("<Data>");

		for (index = 0; index < s->Length; index++)
			WL("%02x", s->Data[index]);

		WL("</Data>\n");
	}
	WL("</Sabun>\n");
}
static void DispSabunList_V(autoList_t *lDiff, autoList_t *rDiff)
{
	Sabun_t *s;
	uint index;

	if (!lDiff && !rDiff)
		return;

	errorCase(!lDiff || !rDiff);
	errorCase(getCount(lDiff) != getCount(rDiff));

	WL("<Sabun-Pair-List>\n");

	foreach (lDiff, s, index)
	{
		WL("<Sabun-Pair>\n");

		DispSabun(s);
		DispSabun((Sabun_t *)getElement(rDiff, index));

		WL("</Sabun-Pair>\n");
	}
	WL("</Sabun-Pair-List>\n");
}
static void DispSabunList(autoList_t *diff, char *tag)
{
	Sabun_t *s;
	uint index;

	if (!diff)
		return;

	WL("<%s>\n", tag);

	foreach (diff, s, index)
		if (s->Length)
			DispSabun(s);

	WL("</%s>\n", tag);
}
static void DispDirSabunList(autoList_t *diff)
{
	DirSabun_t *ds;
	uint index;

	WL("<?xml version=\"1.0\" encoding=\"shift_jis\"?>\n");
	WL("<Dir-Sabun-List>\n");

	foreach (diff, ds, index)
	{
		if (VerboseMode || ds->Kind < 6 || (ds->LDiff && getCount(ds->LDiff)))
		{
			WL("<Dir-Sabun>\n");
			WL("<Path>%s</Path>\n", ds->Path);
			WL("<Is-File>%u</Is-File>\n", ds->Kind & 1);
			WL("<L-Exist>%u</L-Exist>\n", ds->Kind >> 1 & 1);
			WL("<R-Exist>%u</R-Exist>\n", ds->Kind >> 2 & 1);

			if (VerboseMode)
			{
				DispSabunList_V(ds->LDiff, ds->RDiff);
			}
			else
			{
				DispSabunList(ds->LDiff, "L-Diff");
				DispSabunList(ds->RDiff, "R-Diff");
			}
			WL("</Dir-Sabun>\n");
		}
	}
	WL("</Dir-Sabun-List>\n");
}
int main(int argc, char **argv)
{
	WFp = stdout;

	if (argIs("/O"))
	{
		WFp = fileOpen(nextArg(), "wt");
		addFinalizer(CloseWFp);
	}
	if (argIs("/V"))
	{
		VerboseMode = 1;
	}

	if (argIs("/D")) // Dir
	{
		char *localDir;
		char *rDir;
		autoList_t *diff;

		localDir = strx(nextArg());
		rDir = strx(nextArg());

		diff = MakeDirSabun(localDir, rDir, 1, 1);
		DispDirSabunList(diff);
		ReleaseDirSabunList(diff);

		memFree(localDir);
		memFree(rDir);
	}

	if (argIs("/F")) // File
	{
		autoBlock_t *lFData;
		autoBlock_t *rFData;
		autoList_t *lDiff = newList();
		autoList_t *rDiff = newList();

		lFData = readBinary(nextArg());
		rFData = readBinary(nextArg());

		MakeSabun(lFData, rFData, lDiff, rDiff, 1, 1);

		WL("<?xml version=\"1.0\" encoding=\"shift_jis\"?>\n");
		WL("<File-Sabun-List>\n");

		if (VerboseMode)
		{
			DispSabunList_V(lDiff, rDiff);
		}
		else
		{
			DispSabunList(lDiff, "L-Diff");
			DispSabunList(rDiff, "R-Diff");
		}
		WL("</File-Sabun-List>\n");

		releaseBlock(lFData);
		releaseBlock(rFData);
		ReleaseSabunList(lDiff);
		ReleaseSabunList(rDiff);
	}

	termination(0);
}
