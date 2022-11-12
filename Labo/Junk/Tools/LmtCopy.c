#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	char *rDir;
	char *wDir;
	uint lmt;
	autoList_t *files;
	char *file;
	uint index;

	rDir = nextArg();
	wDir = nextArg();
	lmt = toValue(nextArg());

	errorCase(!existDir(rDir));
	errorCase(accessible(wDir));
	errorCase(!m_isRange(lmt, 1, IMAX));

	createDir(wDir);

	files = lsFiles(rDir);
	errorCase(getCount(files) < lmt);
	sortJLinesICase(files);

	setCount(files, lmt);

	foreach (files, file, index)
		copyFile_cx(file, combine(wDir, getLocal(file)));

	// gomi
}
