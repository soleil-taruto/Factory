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
		�ǂݍ��݌��t�@�C��

	destFile
		�o�͐�t�@�C��
		srcFile �Ɠ����p�X�ł����Ă��ǂ��B

	textMode
		�^�̂Ƃ� -> �e�L�X�g���[�h
		�U�̂Ƃ� -> �o�C�i���[���[�h

	uint readElement(FILE *fp)
		�P���R�[�h�ǂݍ��ށB
		����ȏヌ�R�[�h�������Ƃ��� 0 ��Ԃ����ƁB

	void writeElement_x(FILE *fp, uint element)
		�P���R�[�h�������ށB
		�K�v�ł���� element ���J�����邱�ƁB

	sint compElement(uint element1, uint element2)
		element1 �� element2 ���r�������ʂ� strcmp() �I�ɕԂ��B

	partSize
		�������Ɉ�x���ɓǂݍ��߂�u���R�[�h�̍��v�o�C�g���v�̍ő�l�̖ڈ�
		srcFile �̃V�[�N�ʒu�̕ω����o�C�g�Ɋ��Z���Ă��邾���B
		0 �̂Ƃ��͏�ɂP���R�[�h���ɂȂ�B
		�e�p�[�g�̃��R�[�h���� partSize / 100 �𒴂��Ȃ��悤�ɂ���B
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

			// �������̃f�t���O?
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
