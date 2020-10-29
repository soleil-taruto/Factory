#include "MaskGZData.h"

int main(int argc, char **argv)
{
	char *file;
	autoBlock_t *fileData;

	errorCase(!argIs("MASK-GZ-DATA"));

	file = nextArg();
	cout("MaskGZData_file: %s\n", file);
	fileData = readBinary(file);
	LOGPOS();
	MaskGZData(fileData);
	LOGPOS();
	writeBinary(file, fileData);
	LOGPOS();
	releaseAutoBlock(fileData);
	LOGPOS();
}
