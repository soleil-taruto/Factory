/*
	md5.exe [/Q] ...

		/Q ... �W���o�͈ꕔ�}�~

	md5.exe ... /H HEX-STRING

		HEX-STRING -> disp md5

	md5.exe ... /S CP932-STRING

		CP932-STRING -> disp md5

	md5.exe ... /D

		LOOP { dropPath -> disp md5 }

	md5.exe ... /LSS

		found list -> disp md5

	md5.exe ... PATH

		PATH -> disp md5

		�f�B���N�g���̏ꍇ�A�����̃t�@�C���̃n�b�V���ꗗ���o�͂���B
		�T�u�f�B���N�g�����������Ƃ��� dmd5 �R�}���h���g���ĂˁB

	md5.exe ...

		CWD -> disp md5
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\OpenSource\md5.h"

int QuietMode;

static void ShowHashDir(char *dir)
{
	autoList_t *paths = ls(dir);
	autoList_t files;
	char *file;
	uint index;

	files = gndFollowElements(paths, lastDirCount);
	dirFileSort(&files);

	foreach(&files, file, index)
	{
		cout("%s %s\n", c_md5_makeHexHashFile(file), getLocal(file));
	}
	releaseDim(paths, 1);
}
static void ShowHashBlock(autoBlock_t *block)
{
	if(!QuietMode)
	{
		cout("MD5 ( %s ) =\n", c_makeHexLine(block));
	}
	cout("%s\n", c_md5_makeHexHashBlock(block));
}

int main(int argc, char **argv)
{
	if(argIs("/Q"))
	{
		QuietMode = 1;
	}

	if(argIs("/H")) // Hex-string
	{
		ShowHashBlock(c_makeBlockHexLine(nextArg()));
		return;
	}
	if(argIs("/S")) // String
	{
		autoBlock_t gab;
		char *line = nextArg();

		ShowHashBlock(gndBlockLineVar(line, gab));
		return;
	}
	if(argIs("/D")) // Drop
	{
		for(; ; )
		{
			char *path = dropPath();

			if(!path)
				break;

			if(existDir(path))
				ShowHashDir(path);
			else
				cout("%s\n", c_md5_makeHexHashFile(path));

			cout("\n");
			memFree(path);
		}
		return;
	}
	if(argIs("/LSS"))
	{
		autoList_t *files = readLines(FOUNDLISTFILE);
		char *file;
		uint index;

		foreach(files, file, index)
			cout("%s %s\n", c_md5_makeHexHashFile(file), file);

		releaseDim(files, 1);
		return;
	}
	if(hasArgs(1))
	{
		char *path = nextArg();

		if(existDir(path))
		{
			ShowHashDir(path);
			return;
		}
		cout("%s\n", c_md5_makeHexHashFile(path));
		return;
	}
	ShowHashDir(".");
}
