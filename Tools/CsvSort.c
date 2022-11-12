/*
	CsvSort.exe [/S PART-SIZE] [/C SORT-COLUMN-INDEX] [/D] [/I] [/9] (/1 CSV-FILE | INPUT-CSV-FILE OUTPUT-CSV-FILE)
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\CsvSort.h"

int main(int argc, char **argv)
{
	uint partSize = 128 * 1024 * 1024;
	uint sortColumnIndex = 0;
	int sortDirection = 'A';
	int compareMode = 'S';

readArgs:
	if (argIs("/S"))
	{
		partSize = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/C"))
	{
		sortColumnIndex = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/D"))
	{
		sortDirection = 'D';
		goto readArgs;
	}
	if (argIs("/I"))
	{
		compareMode = 'I';
		goto readArgs;
	}
	if (argIs("/9"))
	{
		compareMode = '9';
		goto readArgs;
	}

	if (argIs("/1"))
	{
		CsvSort(getArg(0), getArg(0), sortColumnIndex, sortDirection, compareMode, partSize);
	}
	else
	{
		CsvSort(getArg(0), getArg(1), sortColumnIndex, sortDirection, compareMode, partSize);
	}
}
