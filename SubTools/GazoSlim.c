/*
	�_�u�����������B

	�f�t�H���g�̓���
	�E�K�w�̐󂢕����c���B
	�E�������őO�̕����c���B

	- - -

	GazoSlim.exe [/D] [/R] [�Ώۃf�B���N�g��]

		/D ... �K�w�̐[�������c���B
		/R ... �������Ō�̕����c���B

	- - -
	��

	GazoSlim.exe /D C:\home\�摜
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Progress.h"
#include "C:\Factory\OpenSource\md5.h"

#define MD5_BGN 1
#define MD5_LEN 32

static int KeepPriorDeep; // ? �K�w�̐[�������c���B
static int KeepPriorRear; // ? �������Ō�̕����c���B

static uint GetYenCount(char *str)
{
	uint count = 0;
	char *p;

	for (p = str; *p; p = mbsNext(p))
		if (*p == '\\')
			count++;

	return count;
}
static sint LineComp(uint a, uint b)
{
	char *line1 = (char *)a + MD5_BGN;
	char *line2 = (char *)b + MD5_BGN;
	sint ret;

	ret = memcmp(line1, line2, MD5_LEN);

	if (ret)
		return ret;

	ret = (sint)GetYenCount(line1 + MD5_LEN) - (sint)GetYenCount(line2 + MD5_LEN);

	if (ret)
		return ret * (KeepPriorDeep ? -1 : 1);

	ret = mbs_stricmp(line1 + MD5_LEN, line2 + MD5_LEN);

	if (ret)
		return ret * (KeepPriorRear ? -1 : 1);

	return 0;
}
static sint LineComp_File(uint a, uint b)
{
	char *file1 = (char *)a + MD5_BGN + MD5_LEN;
	char *file2 = (char *)b + MD5_BGN + MD5_LEN;

	return mbs_stricmp(file1, file2);
}
static void DoDeleteFile(char *file)
{
	cout("* %s\n", file);
	semiRemovePath(file);
}
static void GazoSlim(char *dir)
{
	autoList_t *files = lssFiles(dir);
	char *file;
	uint index;
	autoList_t *lines = newList();
	char *line;

	ProgressBegin();

	foreach (files, file, index)
	{
		ProgressRate((double)index / getCount(files));

		addElement(lines, (uint)xcout("_%s%s", c_md5_makeHexHashFile(file), file));
	}
	ProgressEnd(0);

	rapidSort(lines, LineComp);

	foreach (lines, line, index)
	{
		if (!index || _strnicmp(getLine(lines, index - 1) + MD5_BGN, line + MD5_BGN, MD5_LEN))
			line[0] = 'K'; // Keep
		else
			line[0] = 'D'; // Delete
	}
	cout("---- file list ----\n");

	foreach (lines, line, index)
		cout("%s\n", line);

	rapidSort(lines, LineComp_File);

	cout("---- order by file ----\n");

	foreach (lines, line, index)
		cout("%s\n", line);

	cout("---- delete only ----\n");

	foreach (lines, line, index)
		if (line[0] == 'D')
			cout("%s\n", line);

	cout("���s�H\n");

	if (getKey() == 0x1b)
		termination(0);

	cout("���s���܂��B\n");

	foreach (lines, line, index)
		if (line[0] == 'D')
			DoDeleteFile(line + MD5_BGN + MD5_LEN);

	releaseDim(files, 1);
	releaseDim(lines, 1);
}
int main(int argc, char **argv)
{
readArgs:
	if (argIs("/D"))
	{
		KeepPriorDeep = 1;
		goto readArgs;
	}
	if (argIs("/R"))
	{
		KeepPriorRear = 1;
		goto readArgs;
	}

	if (hasArgs(1))
	{
		GazoSlim(nextArg());
		return;
	}

	for (; ; )
	{
		GazoSlim(c_dropDir());
		cout("\n");
	}
}
