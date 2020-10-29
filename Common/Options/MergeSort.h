#pragma once

#include "C:\Factory\Common\all.h"

extern uint (*MS_GetRecordWeightSize)(uint);

void MergeSort(
	char *srcFile,
	char *destFile,
	int textMode,
	uint (*readElement)(FILE *fp),
	void (*writeElement_x)(FILE *fp, uint element),
	sint (*compElement)(uint element1, uint element2),
	uint partSize,
	uint recordConstWeightSize
	);

void MergeSortTextComp(char *srcFile, char *destFile, sint (*funcComp)(char *, char *), uint partSize);
void MergeSortText(char *srcFile, char *destFile, uint partSize);
void MergeSortTextICase(char *srcFile, char *destFile, uint partSize);

void MergeFile(
	char *srcFile1,
	int sorted1,
	char *srcFile2,
	int sorted2,
	char *destFile1,
	char *destFile2,
	char *destFileBoth,
	int textMode,
	uint (*readElement)(FILE *fp),
	void (*writeElement)(FILE *fp, uint element),
	sint (*compElement)(uint element1, uint element2),
	void (*releaseElement)(uint element),
	uint partSize,
	uint recordConstWeightSize
	);

void MergeFileTextComp(char *srcFile1, int sorted1, char *srcFile2, int sorted2, char *destFile1, char *destFile2, char *destFileBoth, sint (*funcComp)(char *, char *), uint partSize);
void MergeFileText(char *srcFile1, int sorted1, char *srcFile2, int sorted2, char *destFile1, char *destFile2, char *destFileBoth, uint partSize);
void MergeFileTextICase(char *srcFile1, int sorted1, char *srcFile2, int sorted2, char *destFile1, char *destFile2, char *destFileBoth, uint partSize);
