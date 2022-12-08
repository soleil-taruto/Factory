#include "C:\Factory\Common\all.h"
#include "libs\Sabun.h"

int main(int argc, char **argv)
{
	int correctAddDelete = 0;

	if (argIs("/CAD")) // Correct Add Delete
	{
		correctAddDelete = 1;
	}

	if (argIs("/M")) // Make sabun
	{
		char *beforeDir;
		char *afterDir;
		char *outDir;
		char *sabunFile;

		cout("+----------------------------+\n");
		cout("| 変更前のフォルダをドロップ |\n");
		cout("+----------------------------+\n");
		beforeDir = dropDir();

		cout("+----------------------------+\n");
		cout("| 変更後のフォルダをドロップ |\n");
		cout("+----------------------------+\n");
		afterDir = dropDir();

		outDir = makeTempDir(NULL);
		sabunFile = combine(outDir, "Sabun.bin");

		makeSabun(sabunFile, beforeDir, afterDir, correctAddDelete);

		execute_x(xcout("START %s", outDir));

		memFree(beforeDir);
		memFree(afterDir);
		memFree(outDir);
		memFree(sabunFile);
	}
	else // 差分適用
	{
		char *targetDir;
		char *sabunFile;

		cout("+----------------------------------+\n");
		cout("| 対象フォルダをドロップ           |\n");
		cout("| * このフォルダの中身を更新します |\n");
		cout("+----------------------------------+\n");
		targetDir = dropDir();

		cout("+------------------------+\n");
		cout("| 差分ファイルをドロップ |\n");
		cout("+------------------------+\n");
		sabunFile = dropFile();

		cout("\n");
		cout("対象フォルダを変更します。\n");
		cout("処理を開始してからはキャンセルできません。\n");
		cout("続行するにはエンターキーを押してね。\n");

		if (clearGetKey() == 0x0d)
		{
			cout("\n");
			cout("アップデートしています...");

			if (sabunUpdate(sabunFile, targetDir) == 0) // ? アップデート対象外だった。
			{
				cout("\r");
				cout("+----------------------------+\n");
				cout("| エラー／アップデート対象外 |\n");
				cout("+----------------------------+\n");
			}
			else
			{
				cout("\rアップデートは完了しました。\n");
			}
			clearWaitKey(5000); // 見えるように
		}
		memFree(targetDir);
		memFree(sabunFile);
	}
}
