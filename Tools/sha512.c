/*
	sha512.exe [/Q] ...

		/Q ... 標準出力一部抑止

	sha512.exe ... /H HEX-STRING

		HEX-STRING <- disp sha512

	sha512.exe ... /S CP932-STRING

		CP932-STRING <- disp sha512

	sha512.exe ... /D

		LOOP { dropPath <- disp sha512 }

	sha512.exe ... PATH

		PATH <- disp sha512

	sha512.exe ...

		CWD <- disp sha512

	ディレクトリの場合、直下のファイルのハッシュ一覧を出力する。
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\OpenSource\sha512.h"

int QuietMode;

static ShowHashDir(char *dir)
{
	autoList_t *paths = ls(dir);
	autoList_t files;
	char *file;
	uint index;

	files = gndFollowElements(paths, lastDirCount);
	dirFileSort(&files);

	foreach (&files, file, index)
	{
		sha512_makeHashFile(file);
		sha512_makeHexHash();
		cout("%s %s\n", sha512_hexHash, getLocal(file));
	}
	releaseDim(paths, 1);
}
static ShowHashBlock(autoBlock_t *block)
{
	if (!QuietMode)
	{
		cout("SHA512 ( %s ) =\n", c_makeHexLine(block));
	}
	sha512_makeHashBlock(block);
	sha512_makeHexHash();
	cout("%s\n", sha512_hexHash);
}

int main(int argc, char **argv)
{
	if (argIs("/Q"))
	{
		QuietMode = 1;
	}

	if (argIs("/H")) // Hex-string
	{
		ShowHashBlock(c_makeBlockHexLine(nextArg()));
		return;
	}
	if (argIs("/S")) // String
	{
		autoBlock_t gab;
		char *line = nextArg();

		ShowHashBlock(gndBlockLineVar(line, gab));
		return;
	}

	if (argIs("/D")) // Drop
	{
		for (; ; )
		{
			char *path = dropPath();

			if (!path)
				break;

			if (existDir(path))
			{
				ShowHashDir(path);
			}
			else
			{
				sha512_makeHashFile(path);
				sha512_makeHexHash();
				cout("%s\n", sha512_hexHash);
			}
			cout("\n");
			memFree(path);
		}
		return;
	}

	if (hasArgs(1))
	{
		char *path = nextArg();

		if (existDir(path))
		{
			ShowHashDir(path);
			return;
		}
		sha512_makeHashFile(path);
		sha512_makeHexHash();
		cout("%s\n", sha512_hexHash);
		return;
	}
	ShowHashDir(".");
}
