/*
	ファイル・ディレクトリのコピー

	----

	zcp.exe [/F] [/M | /R] [/EF] [/B] SOURCE-PATH DESTINATION-PATH

		/F ... 強制モード

			コピー先が既に存在する場合、問い合わせすることなく削除する。

		/M, /R ... 移動モード

			コピー元を削除する。つまり移動する。

		/EF ... 拡張子フィルタ

			コピー先のファイル・ディレクトリ名の拡張子終端の '_' を除去する。

		/B ... コピー先の親ディレクトリを指定する。

			以下は同じ
			zcp /B サンプルデータ out
			zcp サンプルデータ out\サンプルデータ
*/

#include "C:\Factory\Common\all.h"

static void E_Fltr(char *path)
{
	char *ext = getExt(path);

	if (lineExp("<1,,09AZaz>_", ext))
	{
		char *newPath = strx(path);

		strchr(newPath, '\0')[-1] = '\0';

		cout("< %s\n", path);
		cout("> %s\n", newPath);

		moveFile(path, newPath);
		memFree(newPath);
	}
}
int main(int argc, char **argv)
{
	char *srcPath;
	char *destPath;
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
	if (argIs("/EF"))
	{
		ef_mode = 1;
		goto readArgs;
	}
	if (argIs("/B"))
	{
		b_mode = 1;
		goto readArgs;
	}

	srcPath  = nextArg();
	destPath = nextArg();

	/*
		オプションを間違えた？ -> 念のため error
	*/
	errorCase(srcPath[0]  == '/');
	errorCase(destPath[0] == '/');
	errorCase(hasArgs(1));

	srcPath  = makeFullPath(srcPath);
	destPath = makeFullPath(destPath);

	if (b_mode)
		destPath = addLocal(destPath, getLocal(srcPath));

	cout("< %s\n", srcPath);
	cout("> %s\n", destPath);

	if (existPath(destPath))
	{
		if (!force_mode)
		{
			cout("コピー先が存在します。\n");
			cout("削除？\n");

			if (clearGetKey() == 0x1b)
				termination(0);

			cout("削除します。\n");
		}
		recurRemovePath(destPath);
	}

	if (existFile(srcPath))
	{
		createPath(destPath, 'X');

		if (move_mode)
		{
			moveFile(srcPath, destPath);
		}
		else
		{
			copyFile(srcPath, destPath);
		}
	}
	else
	{
		errorCase(!existDir(srcPath));
		createPath(destPath, 'D');

		if (move_mode)
		{
			moveDir(srcPath, destPath);
			removeDir(srcPath);
		}
		else
		{
			copyDir(srcPath, destPath);
		}
	}

	if (ef_mode)
	{
		cout("ef=%s\n", destPath);

		if (existFile(destPath))
		{
			E_Fltr(destPath);
		}
		else
		{
			autoList_t *paths = lss(destPath);
			char *path;
			uint index;

			reverseElements(paths);

			foreach (paths, path, index)
			{
				E_Fltr(path);
			}
			releaseDim(paths, 1);
		}
	}

	memFree(srcPath);
	memFree(destPath);
}
