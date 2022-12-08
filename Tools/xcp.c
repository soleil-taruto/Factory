// smpl
/*
	ディレクトリのコピー

	----

	xcp.exe [/C] [/V [/R]] SOURCE-DIR DESTINATION-DIR

	SOURCE-DIR
		コピー元ディレクトリ
			但し /V 指定時は移動元ディレクトリ
		存在するディレクトリであること。
		変更しない。
			但し /V 指定時は空のディレクトリになる。
				但し /R 指定時は削除される。

	DESTINATION-DIR
		コピー先ディレクトリ
			但し /V 指定時は移動先ディレクトリ
		存在する空のディレクトリであること。(空でない場合は名前が被らないこと)
			但し /C 指定時は存在しない作成可能なパスであること。
*/

#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	char *srcDir;
	char *destDir;
	int create_mode = 0;
	int move_mode = 0;
	int remove_mode = 0;

readArgs:
	if (argIs("/C") || argIs("/MD"))
	{
		create_mode = 1;
		goto readArgs;
	}
	if (argIs("/V") || argIs("/MV"))
	{
		move_mode = 1;
		goto readArgs;
	}
	if (argIs("/R") || argIs("/RM"))
	{
		remove_mode = 1;
		goto readArgs;
	}

	srcDir  = nextArg();
	destDir = nextArg();

	if (create_mode)
		createDir(destDir);

	if (move_mode)
	{
		moveDir(srcDir, destDir);

		if (remove_mode)
			removeDir(srcDir);
	}
	else
	{
		copyDir(srcDir, destDir);
	}
}
