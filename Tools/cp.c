/*
	ファイルのコピー

	----

	cp.exe SOURCE-FILE DESTINATION-PATH

		/F ... 強制モード

			コピー先が既に存在する場合、問い合わせすることなく削除する。

		/M, /R ... 移動モード

			コピー元を削除する。つまり移動する。
*/

#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	char *srcFile;
	char *destFile;
	int force_mode = 0;
	int move_mode = 0;
	int ef_mode = 0;
	int b_mode = 0;

readArgs:
	if (argIs("/F"))
	{
		force_mode = 1;
		goto readArgs;
	}
	if (argIs("/M") || argIs("/R"))
	{
		move_mode = 1;
		goto readArgs;
	}

	srcFile  = nextArg();
	destFile = nextArg();

	/*
		オプションを間違えた？ -> 念のため error
	*/
	errorCase(srcFile[0]  == '/');
	errorCase(destFile[0] == '/');
	errorCase(hasArgs(1));

	srcFile  = makeFullPath(srcFile);
	destFile = makeFullPath(destFile);

	errorCase(!existFile(srcFile));

	if (existDir(destFile))
	{
		destFile = addLocal(destFile, getLocal(srcFile));

		errorCase(existDir(destFile));
	}

	cout("< %s\n", srcFile);
	cout("> %s\n", destFile);

	if (existFile(destFile))
	{
		if (!force_mode)
		{
			cout("コピー先ファイルが存在します。\n");
			cout("削除？\n");

			if (clearGetKey() == 0x1b)
				termination(0);

			cout("削除します。\n");
		}
		recurRemovePath(destFile);
	}

	if (move_mode)
	{
		moveFile(srcFile, destFile);
	}
	else
	{
		copyFile(srcFile, destFile);
	}

	memFree(srcFile);
	memFree(destFile);
}
