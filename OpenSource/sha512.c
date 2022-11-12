#include "sha512.h"

uchar sha512_hash[64];
char sha512_hexHash[129];

sha512_t *sha512_create(void)
{
	sha512_t *i = (sha512_t *)memAlloc(sizeof(sha512_t));

	SHA512_Init(&i->Context);
	return i;
}
void sha512_release(sha512_t *i)
{
	memFree(i);
}
sha512_t *sha512_copy(sha512_t *i)
{
	return (sha512_t *)memClone(i, sizeof(sha512_t));
}
void sha512_update(sha512_t *i, autoBlock_t *text)
{
#if 0 // test
	{
		char *hl = makeHexLine(text);
		cout("%s\n", hl);
		memFree(hl);
	}
#endif
	SHA512_Update(&i->Context, directGetBuffer(text), getSize(text));
}
void sha512_makeHash(sha512_t *i)
{
	static SHA512_CTX ctx;

	ctx = i->Context;
	SHA512_Final(sha512_hash, &ctx);
}
void sha512_makeHexHash(void)
{
	autoBlock_t gab;
	char *line;

	line = makeHexLine(gndBlockVar(sha512_hash, 64, gab));
	strcpy(sha512_hexHash, line);
	memFree(line);
}
void sha512_128_makeHexHash(void)
{
	autoBlock_t gab;
	char *line;

	line = makeHexLine(gndBlockVar(sha512_hash, 16, gab));
	strcpy(sha512_hexHash, line);
	memFree(line);
}

void sha512_makeHashBlock(autoBlock_t *text)
{
	sha512_t *i = sha512_create();

	sha512_update(i, text);
	sha512_makeHash(i);
	sha512_release(i);
}
void sha512_makeHashLine(char *line)
{
	autoBlock_t tmpLine;
	sha512_makeHashBlock(gndBlockLineVar(line, tmpLine));
}
void sha512_makeHashFile(char *file)
{
	FILE *fp = fileOpen(file, "rb");
	sha512_t *i = sha512_create();
	autoBlock_t *block;

	while (block = readBinaryStream(fp, 16 * 1024 * 1024))
	{
		sha512_update(i, block);
		releaseAutoBlock(block);
	}
	fileClose(fp);
	sha512_makeHash(i);
	sha512_release(i);
}

static autoBlock_t *LocalizedBuffer;

void sha512_localize(void)
{
	if (!LocalizedBuffer)
		LocalizedBuffer = newBlock();

	ab_addBlock(LocalizedBuffer, sha512_hash, 64);
	ab_addBlock(LocalizedBuffer, sha512_hexHash, 129);
}
void sha512_unlocalize(void)
{
	errorCase(!LocalizedBuffer);

	unaddBytesToBlock(LocalizedBuffer, sha512_hexHash, 129);
	unaddBytesToBlock(LocalizedBuffer, sha512_hash, 64);
}

char *sha512_128Block(autoBlock_t *block)
{
	sha512_makeHashBlock(block);
	sha512_128_makeHexHash();
	return sha512_hexHash;
}
char *sha512_128Bytes(void *bytes, uint size)
{
	autoBlock_t gab;

	return sha512_128Block(gndBlockVar(bytes, size, gab));
}
char *sha512_128Line(char *line)
{
	autoBlock_t gab;

	return sha512_128Block(gndBlockLineVar(line, gab));
}
char *sha512_128File(char *file)
{
	sha512_makeHashFile(file);
	sha512_128_makeHexHash();
	return sha512_hexHash;
}
