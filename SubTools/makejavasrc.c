#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\MergeSort.h"

static char *DedokoroListFile;

static void AddDedokoro(char *package, char *className, char *file)
{
	FILE *fp;

	if (!DedokoroListFile)
		DedokoroListFile = getOutFile("Dedokoro.txt");

	fp = fileOpen(DedokoroListFile, "at");

	package = strx(package);
	replaceChar(package, '/', '.');

	writeToken(fp, package);
	writeChar(fp, '.');
	writeToken(fp, className);
	writeChar(fp, ' ');
	writeLine(fp, file);

	memFree(package);

	fileClose(fp);
}

static char *CurrClassFile;
static char *CurrJarFile;

static int UnknownFlag;

static char *GetPackage(char *javaFile)
{
	FILE *fp = fileOpen(javaFile, "rb");
	char *ret;

	cout("*P %s\n", javaFile);

	for (; ; )
	{
		char *line = readLine(fp);

		if (!line)
		{
			UnknownFlag = 1;
			ret = strx("_$UnknownPackage");
			break;
		}
		ucTrim(line);

		if (startsWith(line, "package "))
		{
			char *p = line + 8;
			char *q;

			q = strchr(p, ';');
			errorCase(!q);
			*q = '\0';

			ret = strx(p);

			memFree(line);
			break;
		}
		memFree(line);
	}
	fileClose(fp);
	return ret;
}
static char *GetJavaClassName(char *javaFile)
{
	FILE *fp = fileOpen(javaFile, "rb");
	int incomment = 0;
	char *ret;

	cout("*C %s\n", javaFile);

	for (; ; )
	{
		char *line = readLine(fp);
		char *p;

		if (!line)
		{
			UnknownFlag = 1;
			ret = strx("_$UnknownClass");
			break;
		}
		ucTrim(line);

		if (*line == '/') // ? コメント行っぽい。XXX
			*line = '\0';

		if (*line == '*') // ? コメント行っぽい。XXX
			*line = '\0';

		p = strstr(line, "class ");

		if (!p)
			p = strstr(line, "interface ");

		if (!p)
			p = strstr(line, "enum ");

		if (p)
		{
			char *q;

			p = strchr(p, ' ');
			errorCase(!p);
			p++;
			q = strchr(p, ' ');
			if (q) *q = '\0';
			q = strchr(p, '<');
			if (q) *q = '\0';
			q = strchr(p, '{');
			if (q) *q = '\0';

			ret = strx(p);
			trimEdge(ret, ' ');

			memFree(line);
			break;
		}
		memFree(line);
	}
	fileClose(fp);
	return ret;
}
static void ExtractJava(char *javaFile, char *packageRootDir)
{
	char *package;
	char *className;
	char *wFile;

	UnknownFlag = 0;

	package = GetPackage(javaFile);
	className = GetJavaClassName(javaFile);

	replaceChar(package, '.', '/');
	wFile = xcout("%s/%s.java%s", package, className, UnknownFlag ? ".txt" : "");
	restoreYen(wFile);
	wFile = lineToFairRelPath_x(wFile, strlen(packageRootDir));
	wFile = combine_cx(packageRootDir, wFile);

	if (existPath(wFile))
	{
		wFile = addExt(wFile, "_$Collision.txt");
		wFile = toCreatablePath(wFile, UINTMAX);
	}
	createPath(wFile, 'X');
	copyFile(javaFile, wFile);

	if (CurrJarFile)
		AddDedokoro(package, className, CurrJarFile);
	else if (CurrClassFile)
		AddDedokoro(package, className, CurrClassFile);
	else
		AddDedokoro(package, className, javaFile);

	memFree(package);
	memFree(className);
}
static void ExtractClass(char *classFile, char *wDir)
{
	char *file = makeTempPath("java");

	coExecute_x(xcout("C:\\Factory\\SubTools\\jad.exe /DJ \"%s\" \"%s\"", classFile, file));

	CurrClassFile = classFile;

	ExtractJava(file, wDir);

	CurrClassFile = NULL;

	removeFile(file);
	memFree(file);
}
static void ExtractAllJava(char *rDir, char *wDir)
{
	autoList_t *files = lssFiles(rDir);
	char *file;
	uint index;

	sortJLinesICase(files);

	foreach (files, file, index)
		if (!_stricmp("java", getExt(file)))
			ExtractJava(file, wDir);

	releaseDim(files, 1);
}
static void ExtractAllClass(char *rDir, char *wDir)
{
	autoList_t *files = lssFiles(rDir);
	char *file;
	uint index;

	sortJLinesICase(files);

	foreach (files, file, index)
		if (!_stricmp("class", getExt(file)) && !strchr(getLocal(file), '$'))
			ExtractClass(file, wDir);

	releaseDim(files, 1);
}
static void ExtractJar(char *jarFile, char *wDir)
{
	char *dir = makeFreeDir();

	coExecute_x(xcout("C:\\Factory\\SubTools\\zip.exe /X \"%s\" \"%s\"", jarFile, dir));

	addCwd(dir);
	coExecute("ATTRIB.EXE -R /S");
	unaddCwd();

	CurrJarFile = jarFile;

	ExtractAllJava(dir, wDir);
	ExtractAllClass(dir, wDir);

	CurrJarFile = NULL;

//	recurRemoveDir(dir); // utf未対応！
	coExecute_x(xcout("RD /S /Q \"%s\"", dir));

	memFree(dir);
}
static void ExtractAllJar(char *rDir, char *wDir, char *target_ext)
{
	autoList_t *files = lssFiles(rDir);
	char *file;
	uint index;

	sortJLinesICase(files);

	foreach (files, file, index)
		if (!_stricmp(target_ext, getExt(file)))
			ExtractJar(file, wDir);

	releaseDim(files, 1);
}
static void MakeJavaSrc(char *rDir, char *wDir)
{
	rDir = makeFullPath(rDir);
	wDir = makeFullPath(wDir);

	cout("< %s\n", rDir);
	cout("> %s\n", wDir);

	errorCase(!existDir(rDir));
	createPath(wDir, 'X');
	createDirIfNotExist(wDir);

	ExtractAllJava(rDir, wDir);
	ExtractAllJar(rDir, wDir, "zip");
	ExtractAllClass(rDir, wDir);
	ExtractAllJar(rDir, wDir, "jar");

	memFree(rDir);
	memFree(wDir);

	if (DedokoroListFile)
	{
		MergeSortText(DedokoroListFile, DedokoroListFile, 128000000); // 128 MB
		openOutDir();
	}
}
int main(int argc, char **argv)
{
	antiSubversion = 1;
	ignoreUtfPath = 1;

	{
		char *rDir;
		char *wDir;

		rDir = nextArg();
		wDir = nextArg();

		MakeJavaSrc(rDir, wDir);
		return;
	}
}
