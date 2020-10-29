#include "all.h"

/*
	path: ファイル名、ディレクトリでも可
*/
void setFileAttr(char *path, int attrArch, int attrHidden, int attrReadOnly, int attrSystem)
{
	errorCase(!SetFileAttributes(
		path,
		(attrArch     ? FILE_ATTRIBUTE_ARCHIVE  : 0) |
		(attrHidden   ? FILE_ATTRIBUTE_HIDDEN   : 0) |
		(attrReadOnly ? FILE_ATTRIBUTE_READONLY : 0) |
		(attrSystem   ? FILE_ATTRIBUTE_SYSTEM   : 0)
		));
		// ? 失敗
}
