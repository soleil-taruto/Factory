/*
	FOUNDLISTFILE 又はカレントディレクトリの配下にある「同じ内容」のファイルを別の「同じ内容」に編集する。

	--
	コマンド

	ees.exe /LSS

		FOUNDLISTFILE のファイルを編集する。

	ees.exe BASE-FILE

		カレントディレクトリの配下にある BASE-FILE と同じ内容のファイルを編集する。
*/

#include "C:\Factory\Common\all.h"


static void EditSame(char *baseFile, autoList_t *files)
{
	char *escBaseFile = makeTempPath(NULL);
	char *file;
	uint index;

	cout("< %s\n", baseFile);

	foreach(files, file, index)
		cout("> %s\n", file);

	copyFile(baseFile, escBaseFile);

	editTextFile(baseFile);

	if(!existFile(baseFile)) // ? 秀丸によって削除された。-> 復元
		copyFile(escBaseFile, baseFile);

	if(!isSameFile(baseFile, escBaseFile))
	{
		cout("編集を適用します。\n");
		cout("続行？\n");

		if(clearGetKey() == 0x1b)
			termination(0);

		cout("続行します。\n");

		foreach(files, file, index)
		{
			cout("< %s\n", baseFile);
			cout("> %s\n", file);

			semiRemovePath(file);

			copyFile(baseFile, file);
		}
		cout("apply-done\n");
	}
	removeFile(escBaseFile);
	memFree(escBaseFile);
}
static void EditSame_LSS(void)
{
	autoList_t *files = readLines(FOUNDLISTFILE);
	char *file;
	uint index;
	char *baseFile;
	int hasDifferentFile = 0;

	errorCase(getCount(files) < 1);

	sortJLinesICase(files);
	baseFile = (char *)desertElement(files, 0);

	cout("baseFile: %s\n", baseFile);

	errorCase(!existFile(baseFile));

	foreach(files, file, index)
	{
		errorCase(!mbs_stricmp(file, baseFile));
		errorCase(!existFile(file));

		if(!isSameFile(file, baseFile))
		{
			cout("NOT_SAME: %s\n", file);
			hasDifferentFile = 1;
		}
	}

	if(hasDifferentFile)
	{
		cout("内容の異なるファイルが含まれています。\n");
		cout("続行？\n");

		if(clearGetKey() == 0x1b)
			termination(0);

		cout("続行します。\n");
	}

	EditSame(baseFile, files);

	releaseDim(files, 1);
	memFree(baseFile);
}
static void EditSame_File(char *baseFile)
{
	autoList_t *files = lssFiles(".");
	char *file;
	uint index;

	baseFile = makeFullPath(baseFile);

	errorCase_m(findJLineICase(files, baseFile) == getCount(files), "指定されたファイルはカレントディレクトリの配下に在りません。");

	foreach(files, file, index)
		if(!mbs_stricmp(file, baseFile) || !isSameFile(file, baseFile)) // ? baseFile である || baseFile と内容が異なる。-> 除外
			*file = '\0';

	trimLines(files);
	sortJLinesICase(files);

	EditSame(baseFile, files);

	releaseDim(files, 1);
	memFree(baseFile);
}
int main(int argc, char **argv)
{
	antiSubversion = 1;

	if(argIs("/LSS"))
	{
		EditSame_LSS();
	}
	else
	{
		EditSame_File(nextArg());
	}
}
