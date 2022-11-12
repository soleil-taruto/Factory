#include "all.h"

void SortFilesByStamp(autoList_t *files) // files: —v‘f‚ğÄ”z’u‚·‚é‚±‚Æ‚É’ˆÓ
{
	char *file;
	uint index;

	foreach (files, file, index)
	{
		updateFindData(file);
		setElement(files, index, (uint)xcout("%020I64d%s", lastFindData.time_write, file));
		memFree(file);
	}
	sortJLinesICase(files);

	foreach (files, file, index)
	{
		copyLine(file, file + 20);
	}
}
