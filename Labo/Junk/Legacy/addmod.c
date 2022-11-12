/*
	VC2008Ex のプロジェクトにモジュールを追加する。
	ソースファイルのフォルダ (.vcproj が存在するフォルダ) をカレントにして実行すること。

	addmod.exe [MODULE]

		[MODULE] ... <__09AZaz>
*/

#include "C:\Factory\Common\all.h"

static char *GetProjFile(void)
{
	autoList_t *files = lsFiles(".");
	char *file;
	uint index;

	eraseParents(files);

	foreach (files, file, index)
		if (!_stricmp("vcproj", getExt(file)))
			break;

	errorCase(!file); // ? not found

	file = strx(file);
	releaseDim(files, 1);
	return file;
}
static uint FindIndex(autoList_t *lines, uint index, char *format)
{
	while (index < getCount(lines))
	{
		if (lineExp(format, getLine(lines, index)))
			return index;

		index++;
	}
	error(); // ? not found
	return 0; // dummy
}
static void AddModule(char *module)
{
	char *projFile;
	char *srcFile;
	char *hdrFile;
	autoList_t *lines;
	char *line;
	uint index;

	errorCase(!lineExp("<__09AZaz>", module));

	projFile = GetProjFile();
	srcFile = changeExt(module, "cpp");
	hdrFile = changeExt(module, "h");

	errorCase(existPath(srcFile));
	errorCase(existPath(hdrFile));

	lines = readLines(projFile);

	index = FindIndex(lines, 0, "<\t\t  >Name=\"ソース ファイル\"");
	index = FindIndex(lines, index + 1, "<\t\t  >>");
	insertElement(lines, index + 1, (uint)xcout("<File RelativePath=\".\\%s\"></File>", srcFile));

	index = FindIndex(lines, 0, "<\t\t  >Name=\"ヘッダー ファイル\"");
	index = FindIndex(lines, index + 1, "<\t\t  >>");
	insertElement(lines, index + 1, (uint)xcout("<File RelativePath=\".\\%s\"></File>", hdrFile));

	writeLines_xx(projFile, lines);
	addLine2File_cx("all.h", xcout("#include \"%s\"", hdrFile));
	writeOneLine(srcFile, "#include \"all.h\"");
	createFile(hdrFile); // "all" ならここで all.h は空のファイルになる。

	memFree(srcFile);
	memFree(hdrFile);
}
int main(int argc, char **argv)
{
	AddModule(nextArg());
}
