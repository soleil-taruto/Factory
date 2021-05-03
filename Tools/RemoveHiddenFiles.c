/*
	RemoveHiddenFiles.exe [/B] [�Ώ�DIR]
*/

#include "C:\Factory\Common\all.h"

static int BatchMode;

static void RemoveHiddenFiles(char *dir)
{
	autoList_t *files = lssFiles(dir);
	char *file;
	uint index;

	sortJLinesICase(files);

	foreach(files, file, index)
	{
		if(getFileAttr_Hidden(file)) // ? �B���t�@�C�� -> �폜�Ώ�
		{
			cout("%s\n", file);
		}
		else
		{
			*file = '\0'; // ���O
		}
	}
	trimLines(files);

	if(BatchMode)
	{
		cout("���s���܂��B(�o�b�`���[�h)\n");
	}
	else
	{
		// Confirm
		{
			cout("�����B���t�@�C�����폜���܂��B\n");
			cout("���s�H\n");

			if(clearGetKey() == 0x1b)
				termination(0);

			cout("���s���܂��B\n");
		}
	}

	foreach(files, file, index)
		semiRemovePath(file);

	releaseDim(files, 1);
}
int main(int argc, char **argv)
{
	BatchMode = argIs("/B");

	if(hasArgs(1))
	{
		RemoveHiddenFiles(nextArg());
	}
	else
	{
		RemoveHiddenFiles(dropDir()); // g
	}
}
