/*
	jad.exe /D CLASS-FILE

		... CLASS-FILE をデコンパイルして changeExt( CLASS-FILE, "java" ) を生成する。
			インナークラスとか CLASS-FILE のパス名から探す。
			CLASS-FILE のファイル名・拡張子は出鱈目であっても良い。

	jad.exe /DJ CLASS-FILE JAVA-FILE

		... CLASS-FILE をデコンパイルして JAVA-FILE を生成する。
			インナークラスとか CLASS-FILE のパス名から探す。
			CLASS-FILE のファイル名・拡張子は出鱈目であっても良い。
*/

#include "C:\Factory\Common\all.h"

/*
	Jad 1.5.8g for Windows 9x/NT/2000 on Intel platform
	空白を含まないパスであること。
*/
#define JAD_LOCAL_FILE "jad.exe"
#define JAD_FILE "C:\\app\\jad158g.win\\" JAD_LOCAL_FILE

static char *GetJadFile(void)
{
	static char *file;

	if (!file)
	{
		file = JAD_LOCAL_FILE;

		if (!existFile(file))
		{
			file = JAD_FILE;
			errorCase_m(!existFile(file), "[" JAD_FILE "]が見つかりません。");
		}
	}
	return file;
}
static void ClassToJava(char *classFile, char *javaFile)
{
	char *dir = makeTempDir(NULL);

	classFile = makeFullPath(classFile);
	javaFile = makeFullPath(javaFile);

	cout("< %s\n", classFile);
	cout("> %s\n", javaFile);

	errorCase(!existFile(classFile));
	createPath(javaFile, 'F'); // 出力先DIR作成 + 書き込みテスト
	removeFile(javaFile);

	addCwd(dir);
	coExecute_x(xcout("%s \"%s\"", GetJadFile(), classFile));

	{
		autoList_t *files = lsFiles(".");

		if (getCount(files) == 1)
		{
			moveFile(getLine(files, 0), javaFile);
		}
		else
		{
			FILE *fp = fileOpen(javaFile, "wt");
			char *file;
			uint index;

			writeLine(fp, "失敗しました。");
			writeLine_x(fp, xcout("クラスファイル=%s", classFile));

			foreach (files, file, index)
				writeLine_x(fp, xcout("出力ファイル=%s", file));

			fileClose(fp);
		}
		releaseDim(files, 1);
	}

	unaddCwd();

	removeDir(dir);
	memFree(dir);

	memFree(classFile);
	memFree(javaFile);
}
int main(int argc, char **argv)
{
	if (argIs("/D"))
	{
		char *classFile;
		char *javaFile;

		classFile = nextArg();
		javaFile = changeExt(classFile, "java");

		ClassToJava(classFile, javaFile);

		memFree(javaFile);
		return;
	}
	if (argIs("/DJ"))
	{
		char *classFile;
		char *javaFile;

		classFile = nextArg();
		javaFile = nextArg();

		ClassToJava(classFile, javaFile);
		return;
	}
}
