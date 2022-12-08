/*
	*.rum\files に移動して chkrumfiles . と実行する。
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\OpenSource\md5.h"

static int SkipError;

static void CheckFiles(void)
{
	autoList_t *files = lsFiles(".");
	char *file;
	uint index;
	autoList_t *ngFiles = NULL;

	eraseParents(files);

	foreach (files, file, index)
	{
		char *hash = md5_makeHexHashFile(file);

		cout("%s == %s ", file, hash);

		if (SkipError)
		{
			if (!lineExp("<32,09AFaf>", file) || _stricmp(file, hash))
			{
				cout("ng\n");

				if (!ngFiles)
					ngFiles = newList();

				addElement(ngFiles, (uint)makeFullPath(file));
			}
			else
			{
				cout("ok\n");
			}
		}
		else
		{
			errorCase(!lineExp("<32,09AFaf>", file));
			errorCase(_stricmp(file, hash));

			cout("ok\n");
		}
		memFree(hash);
	}
	if (ngFiles)
	{
		cout("★★★ 破損ファイルが %u 件見つかりました。FOUNDLISTFILE に出力します。\n", getCount(ngFiles));

		writeLines(FOUNDLISTFILE, ngFiles);
		releaseDim(ngFiles, 1);
		ngFiles = NULL;
	}
	releaseDim(files, 1);
}
int main(int argc, char **argv)
{
	if (argIs("/-E"))
	{
		SkipError = 1;
	}
	addCwd(hasArgs(1) ? nextArg() : c_dropDir());
	CheckFiles();
	unaddCwd();
}
