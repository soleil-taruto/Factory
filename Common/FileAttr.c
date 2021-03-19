#include "all.h"

/*
	path: �t�@�C�����A�f�B���N�g���ł���
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
		// ? ���s
}
int getFileAttr(char *path, uint mask)
{
	updateFindData(path);
	return (lastFindData.attrib & mask) != 0;
}
int getFileAttr_Arch(char *path)
{
	return getFileAttr(path, _A_ARCH);
}
int getFileAttr_Hidden(char *path)
{
	return getFileAttr(path, _A_HIDDEN);
}
int getFileAttr_ReadOnly(char *path)
{
	return getFileAttr(path, _A_RDONLY);
}
int getFileAttr_System(char *path)
{
	return getFileAttr(path, _A_SYSTEM);
}
