/*
	md5_makeHash() した後でも md5_update() できる。
		この場合、最後の md5_update() からの続きになる。

		md5_t *i = md5_create();
		md5_update(i, text1);
		md5_update(i, text2);
		md5_update(i, text3);
		md5_makeHash(i);      (*1)

		md5_t *i = md5_create();
		md5_update(i, text1);
		md5_makeHash(i);
		md5_update(i, text2);
		md5_makeHash(i);
		md5_update(i, text3);
		md5_makeHash(i);      (*1)と同じハッシュを吐く。
*/

#include "md5.h"

void (*md5_interrupt)(void);

md5_t *md5_create(void)
{
	md5_t *i = (md5_t *)memAlloc(sizeof(md5_t));

	md5_Init(&i->Context);
	return i;
}
void md5_release(md5_t *i)
{
	memFree(i);
}
void md5_update(md5_t *i, autoBlock_t *text)
{
	uint textSize = getSize(text);

	if (textSize)
	{
		md5_Update(&i->Context, directGetBuffer(text), textSize);
	}
}
autoBlock_t *md5_makeHash(md5_t *i)
{
	static md5_CTX ctx;

	ctx = i->Context;
	md5_Final(&ctx);
	return recreateBlock(ctx.digest, 16);
}

autoBlock_t *md5_makeHashBlock(autoBlock_t *text)
{
	md5_t *i = md5_create();
	autoBlock_t *hash;

	md5_update(i, text);
	hash = md5_makeHash(i);
	md5_release(i);

	return hash;
}
autoBlock_t *md5_makeHashLine(char *line)
{
	autoBlock_t tmpList;
	return md5_makeHashBlock(gndBlockLineVar(line, tmpList));
}
autoBlock_t *md5_makeHashFileBuffSize(char *file, uint buffSize)
{
	FILE *fp = fileOpen(file, "rb");
	md5_t *i = md5_create();
	autoBlock_t *hash;

	errorCase(buffSize < 1);

	for (; ; )
	{
		autoBlock_t *buff = readBinaryStream(fp, buffSize);

		if (!buff)
			break;

		if (md5_interrupt)
			md5_interrupt();

		md5_update(i, buff);
		releaseAutoBlock(buff);
	}
	fileClose(fp);

	hash = md5_makeHash(i);
	md5_release(i);

	return hash;
}
autoBlock_t *md5_makeHashFile(char *file)
{
	return md5_makeHashFileBuffSize(file, 16 * 1024 * 1024);
}

// c_
autoBlock_t *c_md5_makeHash(md5_t *i)
{
	static autoBlock_t *stock;
	if (stock) releaseAutoBlock(stock);
	return stock = md5_makeHash(i);
}
autoBlock_t *c_md5_makeHashBlock(autoBlock_t *text)
{
	static autoBlock_t *stock;
	if (stock) releaseAutoBlock(stock);
	return stock = md5_makeHashBlock(text);
}
autoBlock_t *c_md5_makeHashLine(char *line)
{
	static autoBlock_t *stock;
	if (stock) releaseAutoBlock(stock);
	return stock = md5_makeHashLine(line);
}
autoBlock_t *c_md5_makeHashFileBuffSize(char *file, uint buffSize)
{
	static autoBlock_t *stock;
	if (stock) releaseAutoBlock(stock);
	return stock = md5_makeHashFileBuffSize(file, buffSize);
}
autoBlock_t *c_md5_makeHashFile(char *file)
{
	static autoBlock_t *stock;
	if (stock) releaseAutoBlock(stock);
	return stock = md5_makeHashFile(file);
}

// _x
autoBlock_t *md5_makeHashBlock_x(autoBlock_t *text)
{
	autoBlock_t *out = md5_makeHashBlock(text);
	releaseAutoBlock(text);
	return out;
}
autoBlock_t *md5_makeHashLine_x(char *line)
{
	autoBlock_t *out = md5_makeHashLine(line);
	memFree(line);
	return out;
}
autoBlock_t *md5_makeHashFileBuffSize_x(char *file, uint buffSize)
{
	autoBlock_t *out = md5_makeHashFileBuffSize(file, buffSize);
	memFree(file);
	return out;
}
autoBlock_t *md5_makeHashFile_x(char *file)
{
	autoBlock_t *out = md5_makeHashFile(file);
	memFree(file);
	return out;
}

// c_x
autoBlock_t *c_md5_makeHashBlock_x(autoBlock_t *text)
{
	static autoBlock_t *stock;
	if (stock) releaseAutoBlock(stock);
	return stock = md5_makeHashBlock_x(text);
}
autoBlock_t *c_md5_makeHashLine_x(char *line)
{
	static autoBlock_t *stock;
	if (stock) releaseAutoBlock(stock);
	return stock = md5_makeHashLine_x(line);
}
autoBlock_t *c_md5_makeHashFileBuffSize_x(char *file, uint buffSize)
{
	static autoBlock_t *stock;
	if (stock) releaseAutoBlock(stock);
	return stock = md5_makeHashFileBuffSize_x(file, buffSize);
}
autoBlock_t *c_md5_makeHashFile_x(char *file)
{
	static autoBlock_t *stock;
	if (stock) releaseAutoBlock(stock);
	return stock = md5_makeHashFile_x(file);
}

// makeHex*
char *md5_makeHexHash(md5_t *i)
{
	return makeHexLine_x(md5_makeHash(i));
}
char *md5_makeHexHashBlock(autoBlock_t *text)
{
	return makeHexLine_x(md5_makeHashBlock(text));
}
char *md5_makeHexHashLine(char *line)
{
	return makeHexLine_x(md5_makeHashLine(line));
}
char *md5_makeHexHashFileBuffSize(char *file, uint buffSize)
{
	return makeHexLine_x(md5_makeHashFileBuffSize(file, buffSize));
}
char *md5_makeHexHashFile(char *file)
{
	return makeHexLine_x(md5_makeHashFile(file));
}

// c_makeHex*
char *c_md5_makeHexHash(md5_t *i)
{
	static char *stock;
	memFree(stock);
	return stock = md5_makeHexHash(i);
}
char *c_md5_makeHexHashBlock(autoBlock_t *text)
{
	static char *stock;
	memFree(stock);
	return stock = md5_makeHexHashBlock(text);
}
char *c_md5_makeHexHashLine(char *line)
{
	static char *stock;
	memFree(stock);
	return stock = md5_makeHexHashLine(line);
}
char *c_md5_makeHexHashFileBuffSize(char *file, uint buffSize)
{
	static char *stock;
	memFree(stock);
	return stock = md5_makeHexHashFileBuffSize(file, buffSize);
}
char *c_md5_makeHexHashFile(char *file)
{
	static char *stock;
	memFree(stock);
	return stock = md5_makeHexHashFile(file);
}

// makeHex*_x
char *md5_makeHexHashBlock_x(autoBlock_t *text)
{
	char *out = md5_makeHexHashBlock(text);
	releaseAutoBlock(text);
	return out;
}
char *md5_makeHexHashLine_x(char *line)
{
	char *out = md5_makeHexHashLine(line);
	memFree(line);
	return out;
}
char *md5_makeHexHashFileBuffSize_x(char *file, uint buffSize)
{
	char *out = md5_makeHexHashFileBuffSize(file, buffSize);
	memFree(file);
	return out;
}
char *md5_makeHexHashFile_x(char *file)
{
	char *out = md5_makeHexHashFile(file);
	memFree(file);
	return out;
}

// c_makeHex*_x
char *c_md5_makeHexHashBlock_x(autoBlock_t *text)
{
	static char *stock;
	memFree(stock);
	return stock = md5_makeHexHashBlock_x(text);
}
char *c_md5_makeHexHashLine_x(char *line)
{
	static char *stock;
	memFree(stock);
	return stock = md5_makeHexHashLine_x(line);
}
char *c_md5_makeHexHashFileBuffSize_x(char *file, uint buffSize)
{
	static char *stock;
	memFree(stock);
	return stock = md5_makeHexHashFileBuffSize_x(file, buffSize);
}
char *c_md5_makeHexHashFile_x(char *file)
{
	static char *stock;
	memFree(stock);
	return stock = md5_makeHexHashFile_x(file);
}
