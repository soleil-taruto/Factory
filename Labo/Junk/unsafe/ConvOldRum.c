/*
	.rum の昔のハッシュ形式 <md5>-<cnt> を <md5> に変換する。
*/

#include "C:\Factory\Common\all.h"

#define DIR_FILES "files"
#define DIR_REVISIONS "revisions"
#define FILE_FILES "files.txt"

static void ConvOldRum(void)
{
	autoList_t *paths;
	char *path;
	char *newPath;
	uint index;

	errorCase(!existDir(DIR_FILES));
	errorCase(!existDir(DIR_REVISIONS));

	addCwd(DIR_FILES);
	paths = lsFiles(".");
	eraseParents(paths);

	foreach (paths, path, index)
	{
		errorCase(!lineExp("<32,09AZaz>-<1,19><0,,09>", path));

		newPath = strx(path);
		newPath[32] = '\0';
		moveFile(path, newPath);
		memFree(newPath);
	}
	releaseDim(paths, 1);
	unaddCwd();

	addCwd(DIR_REVISIONS);
	paths = lsDirs(".");
	eraseParents(paths);

	foreach (paths, path, index)
	{
		autoList_t *lines;
		char *line;
		uint line_index;

		addCwd(path);
		lines = readLines(FILE_FILES);

		foreach (lines, line, line_index)
		{
			errorCase(!lineExp("<32,09AZaz>-<1,19><0,,09> <>", line));
			eraseLine(line + 32, 2);
		}
		writeLines(FILE_FILES, lines);
		releaseDim(lines, 1);
		unaddCwd();
	}
	releaseDim(paths, 1);
	unaddCwd();
}
int main(int argc, char **argv)
{
	ConvOldRum();
}
