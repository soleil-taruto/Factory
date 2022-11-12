#include "PadFile.h"

#define HASHSIZE 16

void PadFile(char *file, autoBlock_t *footer)
{
	PadFileHash(file);
	PadFileFooter(file, footer);
}
int UnpadFile(char *file, autoBlock_t *footer)
{
	return
		UnpadFileFooter(file, footer) &&
		UnpadFileHash(file);
}
void PadFileHash(char *file)
{
	autoBlock_t *hash = md5_makeHashFile(file);

	writeJoinBinary(file, hash);
	releaseAutoBlock(hash);
}
int UnpadFileHash(char *file)
{
	uint64 size = getFileSize(file);
	uint retval = 0;

	if (HASHSIZE <= size)
	{
		FILE *fp = fileOpen(file, "rb");
		autoBlock_t *rHash;
		autoBlock_t *hash;

		fileSeek(fp, SEEK_SET, size - HASHSIZE);
		rHash = neReadBinaryBlock(fp, HASHSIZE);
		fileClose(fp);
		setFileSize(file, size - HASHSIZE);
		hash = md5_makeHashFile(file);

		retval = isSameBlock(rHash, hash);

		releaseAutoBlock(rHash);
		releaseAutoBlock(hash);
	}
	return retval;
}
void PadFileFooter(char *file, autoBlock_t *footer)
{
	writeJoinBinary(file, footer);
}
int UnpadFileFooter(char *file, autoBlock_t *footer)
{
	uint64 size = getFileSize(file);
	uint retval = 0;

	if (getSize(footer) <= size)
	{
		FILE *fp = fileOpen(file, "rb");
		autoBlock_t *rFooter;

		fileSeek(fp, SEEK_SET, size - getSize(footer));
		rFooter = neReadBinaryBlock(fp, getSize(footer));
		fileClose(fp);
		setFileSize(file, size - getSize(footer));

		retval = isSameBlock(rFooter, footer);

		releaseAutoBlock(rFooter);
	}
	return retval;
}

void PadFileLine(char *file, char *footer)
{
	autoBlock_t gab;
	PadFile(file, gndBlockLineVar(footer, gab));
}
int UnpadFileLine(char *file, char *footer)
{
	autoBlock_t gab;
	return UnpadFile(file, gndBlockLineVar(footer, gab));
}
void PadFileFooterLine(char *file, char *footer)
{
	autoBlock_t gab;
	PadFileFooter(file, gndBlockLineVar(footer, gab));
}
int UnpadFileFooterLine(char *file, char *footer)
{
	autoBlock_t gab;
	return UnpadFileFooter(file, gndBlockLineVar(footer, gab));
}

static autoBlock_t *PF2_GetHash(char *file, char *uniqueLabel)
{
	autoBlock_t *hash = md5_makeHashFile(file);
	ab_addBytes_x(hash, md5_makeHashLine(uniqueLabel));
	return md5_makeHashBlock_x(hash);
}
void PadFile2(char *file, char *uniqueLabel)
{
	autoBlock_t *hash = PF2_GetHash(file, uniqueLabel);

	writeJoinBinary(file, hash);
	releaseAutoBlock(hash);
}
int UnpadFile2(char *file, char *uniqueLabel)
{
	uint64 size = getFileSize(file);
	uint retval = 0;

	if (HASHSIZE <= size)
	{
		FILE *fp = fileOpen(file, "rb");
		autoBlock_t *rHash;
		autoBlock_t *hash;

		fileSeek(fp, SEEK_SET, size - HASHSIZE);
		rHash = neReadBinaryBlock(fp, HASHSIZE);
		fileClose(fp);
		setFileSize(file, size - HASHSIZE);
		hash = PF2_GetHash(file, uniqueLabel);

		retval = isSameBlock(rHash, hash);

		releaseAutoBlock(rHash);
		releaseAutoBlock(hash);
	}
	return retval;
}
