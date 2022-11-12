#include "MergeSort_v1.h"

static void CommitPart(
	autoList_t *partFiles,
	char *wMode,
	void (*writeElement_x)(FILE *fp, uint element),
	sint (*compElement)(uint element1, uint element2),
	autoList_t *elements
	)
{
	rapidSort(elements, compElement);

	{
		char *partFile = makeTempPath("part");
		FILE *fp;
		uint element;
		uint index;

		addElement(partFiles, (uint)partFile);
		fp = fileOpen(partFile, wMode);

		foreach (elements, element, index)
			writeElement_x(fp, element);

		fileClose(fp);
		releaseAutoList(elements);
	}
}
static void MergePart(
	char *srcFile1,
	char *srcFile2,
	char *destFile,
	char *rMode,
	char *wMode,
	uint (*readElement)(FILE *fp),
	void (*writeElement_x)(FILE *fp, uint element),
	sint (*compElement)(uint element1, uint element2)
	)
{
	FILE *rfp1 = fileOpen(srcFile1, rMode);
	FILE *rfp2 = fileOpen(srcFile2, rMode);
	FILE *wfp = fileOpen(destFile, wMode);
	uint element1;
	uint element2;

	element1 = readElement(rfp1);
	element2 = readElement(rfp2);

	while (element1 && element2)
	{
		if (compElement(element1, element2) <= 0) // ? element1 <= element2
		{
			writeElement_x(wfp, element1);
			element1 = readElement(rfp1);
		}
		else // element1 > element2
		{
			writeElement_x(wfp, element2);
			element2 = readElement(rfp2);
		}
	}

	{
		FILE *tfp;
		uint tElement;

		if (element1)
		{
			tfp = rfp1;
			tElement = element1;
		}
		else
		{
			tfp = rfp2;
			tElement = element2;
		}

		do
		{
			writeElement_x(wfp, tElement);
		}
		while (tElement = readElement(tfp));
	}

	fileClose(rfp1);
	fileClose(rfp2);
	fileClose(wfp);

	removeFile(srcFile1);
	removeFile(srcFile2);
}
/*
	srcFile
		読み込み元ファイル

	destFile
		出力先ファイル
		srcFile と同じパスであっても良い。

	textMode
		真のとき -> テキストモード
		偽のとき -> バイナリーモード

	uint readElement(FILE *fp)
		１レコード読み込む。
		これ以上レコードが無いときは 0 を返すこと。

	void writeElement_x(FILE *fp, uint element)
		１レコード書き込む。
		必要であれば element を開放すること。

	sint compElement(uint element1, uint element2)
		element1 と element2 を比較した結果を strcmp() 的に返す。

	partSize
		メモリに一度期に読み込める「レコードの合計バイト数」の最大値の目安
		srcFile のシーク位置の変化をバイトに換算しているだけ。
		0 のときは常に１レコードずつになる。
		各パートのレコード数が partSize / 100 を超えないようにする。
*/
void MergeSort(
	char *srcFile,
	char *destFile,
	int textMode,
	uint (*readElement)(FILE *fp),
	void (*writeElement_x)(FILE *fp, uint element),
	sint (*compElement)(uint element1, uint element2),
	uint partSize
	)
{
	autoList_t *partFiles = newList();
	autoList_t *elements = NULL;
	char *rMode;
	char *wMode;
	FILE *fp;
	uint64 startPos = 0;

	if (textMode)
	{
		rMode = "rt";
		wMode = "wt";
	}
	else
	{
		rMode = "rb";
		wMode = "wb";
	}
	fp = fileOpen(srcFile, rMode);

	for (; ; )
	{
		uint element = readElement(fp);
		uint64 currPos;

		if (!element)
			break;

		if (!elements)
			elements = createAutoList(partSize / 100 + 1);

		addElement(elements, element);

		currPos = _ftelli64(fp);
		errorCase(currPos < 0);

		if (startPos + partSize <= currPos || partSize / 100 < getCount(elements))
		{
			CommitPart(partFiles, wMode, writeElement_x, compElement, elements);
			elements = NULL;
			startPos = currPos;

			// メモリのデフラグ?
			{
				char *wkFile = makeTempPath("work");

				writeLines_cx(wkFile, partFiles);
				partFiles = readLines(wkFile);

				removeFile(wkFile);
				memFree(wkFile);
			}
		}
	}
	if (elements)
		CommitPart(partFiles, wMode, writeElement_x, compElement, elements);

	fileClose(fp);

	while (2 < getCount(partFiles))
	{
		char *partFile1 = (char *)unaddElement(partFiles);
		char *partFile2 = (char *)unaddElement(partFiles);
		char *partFile3 = makeTempPath("part");

		MergePart(partFile1, partFile2, partFile3, rMode, wMode, readElement, writeElement_x, compElement);

		memFree(partFile1);
		memFree(partFile2);

		insertElement(partFiles, 0, (uint)partFile3);
	}
	switch (getCount(partFiles))
	{
	case 2:
		MergePart(getLine(partFiles, 0), getLine(partFiles, 1), destFile, rMode, wMode, readElement, writeElement_x, compElement);
		break;

	case 1:
		removeFileIfExist(destFile);
		moveFile(getLine(partFiles, 0), destFile);
		break;

	case 0:
		createFile(destFile);
		break;

	default:
		error();
	}
	releaseDim(partFiles, 1);
}

void MergeSortTextComp(char *srcFile, char *destFile, sint (*funcComp)(char *, char *), uint partSize)
{
	MergeSort(srcFile, destFile, 1, (uint (*)(FILE *))readLine, (void (*)(FILE *, uint))writeLine_x, (sint (*)(uint, uint))funcComp, partSize);
}
void MergeSortText(char *srcFile, char *destFile, uint partSize)
{
	MergeSortTextComp(srcFile, destFile, strcmp, partSize);
}
