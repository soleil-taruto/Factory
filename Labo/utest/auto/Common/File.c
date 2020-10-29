#include "C:\Factory\Common\all.h"

static char *GetStrFiles(char *dir)
{
	autoList_t *files;
	char *ret;

	dir = makeFullPath(dir);
	files = lss(dir);
	changeRoots(files, dir, NULL);
	sortJLinesICase(files);
	ret = untokenize(files, ":");

	memFree(dir);
	releaseDim(files, 1);

//	cout("ret: %s\n", ret);

	return ret;
}
static char *c_GetStrFiles(char *dir)
{
	static char *stock;
	memFree(stock);
	stock = GetStrFiles(dir);
	return stock;
}

static void Test_removeDir(void)
{
	char *dir = makeTempDir(NULL);

	addCwd(dir);
	{
		createDir("a");

		errorCase(_stricmp("a", c_GetStrFiles(".")));

		removeDir("a");

		errorCase(_stricmp("", c_GetStrFiles(".")));

		createDir("a");
		createDir("b");
		createDir("c");

		errorCase(_stricmp("a:b:c", c_GetStrFiles(".")));

		removeDir("a");
		removeDir("b");
		removeDir("c");

		errorCase(_stricmp("", c_GetStrFiles(".")));
	}
	unaddCwd();

	removeDir_x(dir);
	cout("OK\n");
}
static void Test_removeFile(void)
{
	char *dir = makeTempDir(NULL);

	addCwd(dir);
	{
		createFile("a");

		errorCase(_stricmp("a", c_GetStrFiles(".")));

		removeFile("a");

		errorCase(_stricmp("", c_GetStrFiles(".")));

		createFile("a");
		createFile("b");
		createFile("c");

		errorCase(_stricmp("a:b:c", c_GetStrFiles(".")));

		removeFile("a");
		removeFile("b");
		removeFile("c");

		errorCase(_stricmp("", c_GetStrFiles(".")));
	}
	unaddCwd();

	removeDir_x(dir);
	cout("OK\n");
}
static void Test_renameFile(void)
{
	char *dir = makeTempDir(NULL);

	addCwd(dir);
	{
		createFile("abc");

		errorCase(_stricmp("abc", c_GetStrFiles(".")));

		moveFile("abc", "def");

		errorCase(_stricmp("def", c_GetStrFiles(".")));

		removeFile("def");

		errorCase(_stricmp("", c_GetStrFiles(".")));
	}
	unaddCwd();

	removeDir_x(dir);
	cout("OK\n");
}

int main(int argc, char **argv)
{
	Test_removeDir();
	Test_removeFile();
	Test_renameFile();
}
