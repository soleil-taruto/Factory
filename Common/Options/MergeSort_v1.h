#pragma once

#include "C:\Factory\Common\all.h"

void MergeSort(
	char *srcFile,
	char *destFile,
	int textMode,
	uint (*readElement)(FILE *fp),
	void (*writeElement_x)(FILE *fp, uint element),
	sint (*compElement)(uint element1, uint element2),
	uint partSize
	);

void MergeSortTextComp(char *srcFile, char *destFile, sint (*funcComp)(char *, char *), uint partSize);
void MergeSortText(char *srcFile, char *destFile, uint partSize);
