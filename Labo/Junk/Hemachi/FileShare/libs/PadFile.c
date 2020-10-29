#include "all.h"

#define PAD_LABEL_DL "Hemachi_FileShare_DL"
#define PAD_LABEL_UP "Hemachi_FileShare_UP"

void HFS_PadFile(char *file, int dlMode)
{
	PadFile2(file, dlMode ? PAD_LABEL_DL : PAD_LABEL_UP);
}
int HFS_UnpadFile(char *file, int dlMode)
{
	return UnpadFile2(file, dlMode ? PAD_LABEL_DL : PAD_LABEL_UP);
}
