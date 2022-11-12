#include "C:\Factory\Common\all.h"

static void FileToStream(char *rFile, FILE *wFp)
{
	FILE *rFp = fileOpen(rFile, "rb");

	LOGPOS();

	for (; ; )
	{
		autoBlock_t *buff = readBinaryStream(rFp, 512000000);

		if (!buff)
			break;

		LOGPOS();
		writeBinaryBlock_x(wFp, buff);
		LOGPOS();
	}
	fileClose(rFp);
	LOGPOS();
}
static void DoJoin(char *rDir, char *outFile)
{
	autoList_t *files;
	char *file;
	uint index;
	FILE *fp;

	cout("> %s\n", outFile);

	removeFileIfExist(outFile);

	files = lsFiles(rDir);
	sortJLinesICase(files);

	fp = fileOpen(outFile, "wb");

	foreach (files, file, index)
	{
		cout("< %s\n", file);

//		writeBinaryBlock_x(fp, readBinary(file)); // ‚Å‚©‰ß‚¬‚Ä–³—
		FileToStream(file, fp);

		removeFile(file);
	}
	releaseDim(files, 1);
	fileClose(fp);

	cout("\\e\n");
}
int main(int argc, char **argv)
{
	DoJoin(getArg(0), getArg(1));
}
