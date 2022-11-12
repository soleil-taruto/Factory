/*
	鯖は１画面
	ログオン画面は画像、デスクトップは単色なので、
	スクリーンキャプチャした画像の (0, 0) ～ (99, 0) が全て同じ色であればデスクトップと見なす。
	デスクトップの左上にごみ箱があるはず。
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Meteor\Toolkit.h"
#include "C:\Factory\SubTools\libs\bmp.h"

static int IsSingleColor_First100Px_BmpRow(autoList_t *bmpRow)
{
	uint x;

	for (x = 1; x < 100; x++)
		if (refElement(bmpRow, 0) != refElement(bmpRow, x))
			return 0;

	return 1;
}
static int IsLoggedOn(void)
{
	char *dir = makeTempDir(NULL);
	int ret = 0;

	addCwd(dir);
	{
		coExecute(FILE_TOOLKIT_EXE " /PRINT-SCREEN ss_");

		if (existFile("ss_01.bmp")) // ファイルが無い == スクリーンショットを撮れない場合は、パスワード入力中 (Secure Desktop) と考えられる。
		{
			autoList_t *bmp = readBMPFile("ss_01.bmp");
			autoList_t *bmpRow;
			uint x;

			LOGPOS();
			bmpRow = refList(bmp, 0);

			if (IsSingleColor_First100Px_BmpRow(bmpRow))
			{
				LOGPOS();
				bmpRow = refList(bmp, 35); // 左上にはごみ箱があるはず。無ければ (この段も単色なら) ブランク画面かも？

				if (!IsSingleColor_First100Px_BmpRow(bmpRow))
				{
					LOGPOS();
					ret = 1;
				}
			}
			releaseDim_BR(bmp, 2, NULL);
		}
	}
	unaddCwd();

	recurRemoveDir_x(dir);
	return ret;
}
int main(int argc, char **argv)
{
	uint exitCode = 0;

	errorCase(!existFile(FILE_TOOLKIT_EXE)); // 外部コマンド存在確認

	while (!IsLoggedOn())
	{
		int key = coWaitKey(5000);

		if (key == 0x0d)
			break;

		if (key == 0x1b)
		{
			exitCode = 1;
			break;
		}
		clearKey();
	}
	cout("exitCode: %u\n", exitCode);
	termination(exitCode);
}
