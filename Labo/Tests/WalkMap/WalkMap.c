/*
	WalkMap.exe [MAPBMP]

		MAPBMP ... C:\\temp\\A83Map.bmp など、Actor83のデバッグ出力
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\SubTools\libs\bmp.h"

#define FIELD_COLOR 0x0000ff

#define GetColor(cR, cG, cB) \
	((cR) * 65536 + (cG) * 256 + (cB))

static autoList_t *Bmp;
static uint Bmp_W;
static uint Bmp_H;

static uint *DotAt(uint x, uint y)
{
	errorCase(Bmp_W <= x);
	errorCase(Bmp_H <= y);

	return directGetPoint(getList(Bmp, y), x);
}

static uint StartX;
static uint StartY;

static void FindStartPos(void)
{
	uint x;
	uint y;

	for (x = 0; x < Bmp_W; x++)
	{
		if (*DotAt(x, 0) == FIELD_COLOR)
		{
			StartX = x;
			StartY = 0;
			return;
		}
		if (*DotAt(x, Bmp_H - 1) == FIELD_COLOR)
		{
			StartX = x;
			StartY = Bmp_H - 1;
			return;
		}
	}
	for (y = 0; y < Bmp_H; y++)
	{
		if (*DotAt(0, y) == FIELD_COLOR)
		{
			StartX = 0;
			StartY = y;
			return;
		}
		if (*DotAt(Bmp_W - 1, y) == FIELD_COLOR)
		{
			StartX = Bmp_W - 1;
			StartY = y;
			return;
		}
	}
	error(); // not found
}

static autoList_t *PaceMap;

static uint *PaceAt(uint x, uint y)
{
	errorCase(Bmp_W <= x);
	errorCase(Bmp_H <= y);

	return directGetPoint(getList(PaceMap, y), x);
}
static void MakePaceMap(void)
{
	autoList_t *walkCountList = newList();
	autoList_t *walkXPosList = newList();
	autoList_t *walkYPosList = newList();

	PaceMap = makeTable(Bmp_H, Bmp_W, UINTMAX);

	addElement(walkCountList, 0);
	addElement(walkXPosList, StartX);
	addElement(walkYPosList, StartY);

	while (getCount(walkCountList))
	{
		uint w = desertElement(walkCountList, 0);
		uint x = desertElement(walkXPosList, 0);
		uint y = desertElement(walkYPosList, 0);

		if (
			x < Bmp_W &&
			y < Bmp_H &&
			*DotAt(x, y) == FIELD_COLOR &&
			w < *PaceAt(x, y)
			)
		{
			*PaceAt(x, y) = w;

			addElement(walkCountList, w + 1);
			addElement(walkCountList, w + 1);
			addElement(walkCountList, w + 1);
			addElement(walkCountList, w + 1);

			addElement(walkXPosList, x - 1);
			addElement(walkXPosList, x + 1);
			addElement(walkXPosList, x);
			addElement(walkXPosList, x);

			addElement(walkYPosList, y);
			addElement(walkYPosList, y);
			addElement(walkYPosList, y - 1);
			addElement(walkYPosList, y + 1);
		}
	}
	releaseAutoList(walkCountList);
	releaseAutoList(walkXPosList);
	releaseAutoList(walkYPosList);
}

static autoList_t *UnaccessXPosList;
static autoList_t *UnaccessYPosList;

static void FUP_Paint(autoList_t *uaMap, uint x, uint y)
{
	autoList_t *xLst = newList();
	autoList_t *yLst = newList();

	addElement(xLst, x);
	addElement(yLst, y);

	while (getCount(xLst))
	{
		x = desertElement(xLst, 0);
		y = desertElement(yLst, 0);

		if (
			x < Bmp_W &&
			y < Bmp_H &&
			*getTablePoint(uaMap, x, y)
			)
		{
			*getTablePoint(uaMap, x, y) = 0;

			addElement(xLst, x - 1);
			addElement(xLst, x + 1);
			addElement(xLst, x);
			addElement(xLst, x);

			addElement(yLst, y);
			addElement(yLst, y);
			addElement(yLst, y - 1);
			addElement(yLst, y + 1);
		}
	}
	releaseAutoList(xLst);
	releaseAutoList(yLst);
}
static void FindUnaccessPos(void)
{
	autoList_t *uaMap = makeTable(Bmp_W, Bmp_H, 0);
	uint x;
	uint y;

	UnaccessXPosList = newList();
	UnaccessYPosList = newList();

	for (x = 0; x < Bmp_W; x++)
	for (y = 0; y < Bmp_H; y++)
	{
		if (*PaceAt(x, y) == UINTMAX && *DotAt(x, y) == FIELD_COLOR)
		{
			*getTablePoint(uaMap, x, y) = 1;
		}
	}
	for (x = 0; x < Bmp_W; x++)
	for (y = 0; y < Bmp_H; y++)
	{
		if (*getTablePoint(uaMap, x, y))
		{
			addElement(UnaccessXPosList, x);
			addElement(UnaccessYPosList, y);

			FUP_Paint(uaMap, x, y);
		}
	}
	releaseDim_BR(uaMap, 2, NULL);
}

#define PACEMAP_BMPFILE "PaceMap.bmp"
#define UNACCESS_BMPFILE "Unaccess.bmp"
#define UNACCESS_FILE "Unaccess.txt"

static autoList_t *OPM_ColorMap;
static uint OPM_ColorPerPace;

static void OPM_SetColorMap(void)
{
	autoList_t *cm = newList();
	uint c;

#if 1
	for (c = 0; c < 255; c++)
	{
		addElement(cm, GetColor(c + 0, c + 0, 255 - c));
		addElement(cm, GetColor(c + 1, c + 0, 255 - c));
		addElement(cm, GetColor(c + 1, c + 1, 255 - c));
	}
	addElement(cm, GetColor(255, 255, 0));
#else
	for (c = 0; c < 255; c++) addElement(cm, GetColor(0, c, 255 - c));
	for (c = 0; c < 255; c++) addElement(cm, GetColor(c, 255 - c, 0));
	for (c = 0; c < 255; c++) addElement(cm, GetColor(255 - c, c, c));
	for (c = 0; c < 255; c++) addElement(cm, GetColor(c, 255 - c, 255));
	for (c = 0; c < 255; c++) addElement(cm, GetColor(255, c, 255 - c));
	for (c = 0; c < 255; c++) addElement(cm, GetColor(255, 255, c));
	for (c = 0; c < 255; c++) addElement(cm, GetColor(255 - c, 255 - c, 255));
#endif

	OPM_ColorMap = cm;
}
static void OPM_SetColorPerPace(void)
{
	uint maxPace = 0;
	uint x;
	uint y;

	for (x = 0; x < Bmp_W; x++)
	for (y = 0; y < Bmp_H; y++)
	{
		uint pace = *PaceAt(x, y);

		if (pace != UINTMAX)
		{
			m_maxim(maxPace, pace);
		}
	}

	OPM_ColorPerPace = maxPace / getCount(OPM_ColorMap) + 1;
}
static uint OPM_PaceToColor(uint pace)
{
	if (pace == UINTMAX)
		return 0;

	return getElement(OPM_ColorMap, (pace / OPM_ColorPerPace) % getCount(OPM_ColorMap));
}
static void OutputPaceMap(void)
{
	autoList_t *pm_bmp = makeTable(Bmp_H, Bmp_W, 0);
	uint x;
	uint y;

	OPM_SetColorMap();
	OPM_SetColorPerPace();

	for (x = 0; x < Bmp_W; x++)
	for (y = 0; y < Bmp_H; y++)
	{
		setElement(getList(pm_bmp, y), x, OPM_PaceToColor(*PaceAt(x, y)));
	}
	writeBMPFile_cx(c_getOutFile(PACEMAP_BMPFILE), pm_bmp);
}
static void OutputUnaccessMap(void)
{
	autoList_t *um_bmp = makeTable(Bmp_H, Bmp_W, 0);
	uint x;
	uint y;

	for (x = 0; x < Bmp_W; x++)
	for (y = 0; y < Bmp_H; y++)
	{
		uint ua = *PaceAt(x, y) == UINTMAX && *DotAt(x, y) == FIELD_COLOR;

		setElement(getList(um_bmp, y), x, ua ? 0xffffff : 0);
	}
	writeBMPFile_cx(c_getOutFile(UNACCESS_BMPFILE), um_bmp);
}
static void OutputUnaccessPos(void)
{
	FILE *fp = fileOpen(c_getOutFile(UNACCESS_FILE), "wt");
	uint index;

	for (index = 0; index < getCount(UnaccessXPosList); index++)
	{
		uint x = getElement(UnaccessXPosList, index);
		uint y = getElement(UnaccessYPosList, index);

		writeLine_x(fp, xcout("\t\tGUPL_POS(%u, %u);", x, y));
	}
}
static void WalkMap(char *bmpFile)
{
	Bmp = readBMPFile(bmpFile);
	Bmp_W = getCount(getList(Bmp, 0));
	Bmp_H = getCount(Bmp);

LOGPOS();
	FindStartPos();
LOGPOS();
	MakePaceMap();
LOGPOS();
	FindUnaccessPos();
LOGPOS();
	OutputPaceMap();
LOGPOS();
	OutputUnaccessMap();
LOGPOS();
	OutputUnaccessPos();
LOGPOS();

	releaseDim_BR(Bmp, 2, NULL);

	openOutDir();
}
int main(int argc, char **argv)
{
	if (argIs("/D"))
	{
		WalkMap(c_dropFile());
		return;
	}
	if (hasArgs(1))
	{
		WalkMap(nextArg());
		return;
	}
}
