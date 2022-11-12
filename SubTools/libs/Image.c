#include "Image.h"

#define IMG2CSV_EXE FILE_BMPTOCSV_EXE

static char *GetImg2CsvExe(void)
{
	return GetCollaboFile(IMG2CSV_EXE);
}

static uint ImageId;

static autoTable_t *GetImage(void)
{
	static autoTable_t *imgs[IMAGE_ID_NUM];

	if (!imgs[ImageId])
		imgs[ImageId] = newTable(getZero, noop_u);

	return imgs[ImageId];
}
void SetImageId(uint id)
{
	errorCase(IMAGE_ID_NUM <= id);
	ImageId = id;
}

static uint JpegQuality = 90; // 0 ` 100 == ’á•iŽ¿ ` ‚•iŽ¿

void SetImageJpegQuality(uint quality)
{
	errorCase(100 < quality);
	JpegQuality = quality;
}

void ConvImageFile(char *rFile, char *wFile)
{
	errorCase(m_isEmpty(rFile));
	errorCase(m_isEmpty(wFile));

	removeFileIfExist(wFile);

	errorCase(!existFile(rFile));
	errorCase(existFile(wFile));

	execute_x(xcout("START \"\" /B /WAIT \"%s\" /J %u /- \"%s\" \"%s\"", GetImg2CsvExe(), JpegQuality, rFile, wFile));

//	errorCase(!existFile(rFile));
	errorCase(!existFile(wFile));
}
void LoadImageFile(char *rFile)
{
	char *csvFile = makeTempPath("csv");
	autoList_t *csvTable;
	autoList_t *row;
	uint rowidx;
	char *cell;
	uint colidx;

	LOGPOS();
	ConvImageFile(rFile, csvFile);
	LOGPOS();

	csvTable = readCSVFile(csvFile);
	resizeTable(GetImage(), getCount(getList(csvTable, 0)), getCount(csvTable));

	foreach (csvTable, row, rowidx)
	foreach (row, cell, colidx)
	{
		setTableCell(GetImage(), colidx, rowidx, toValueDigits(cell, hexadecimal));
	}
	releaseDim(csvTable, 2);
	removeFile(csvFile);
	memFree(csvFile);
	LOGPOS();
}
void SaveImageFile(char *wFile)
{
	char *csvFile = makeTempPath("csv");
	autoList_t *rows = newList();
	uint rowidx;

	LOGPOS();

	for (rowidx = 0; rowidx < getTableHeight(GetImage()); rowidx++)
	{
		autoList_t *row = newList();
		uint colidx;

		for (colidx = 0; colidx < getTableWidth(GetImage()); colidx++)
		{
			char *cell = xcout("%08x", getTableCell(GetImage(), colidx, rowidx));
			addElement(row, (uint)cell);
		}
		addElement(rows, (uint)row);
	}
	writeCSVFile(csvFile, rows);

	LOGPOS();
	ConvImageFile(csvFile, wFile);
	LOGPOS();

	releaseDim(rows, 2);
	removeFile(csvFile);
	memFree(csvFile);
}

void SetImageSize(uint w, uint h)
{
	resizeTable(GetImage(), w, h);
}
uint GetImg_W(void)
{
	return getTableWidth(GetImage());
}
uint GetImg_H(void)
{
	return getTableHeight(GetImage());
}

uint Img_A;
uint Img_R;
uint Img_G;
uint Img_B;

void GetImgDot(uint x, uint y)
{
	uint dot = getTableCell(GetImage(), x, y);

	Img_A = dot >> 24 & 0xff;
	Img_R = dot >> 16 & 0xff;
	Img_G = dot >>  8 & 0xff;
	Img_B = dot >>  0 & 0xff;
}
void SetImgDot(uint x, uint y)
{
	uint dot =
		(Img_A & 0xff) << 24 |
		(Img_R & 0xff) << 16 |
		(Img_G & 0xff) <<  8 |
		(Img_B & 0xff) <<  0;

	setTableCell(GetImage(), x, y, dot);
}
