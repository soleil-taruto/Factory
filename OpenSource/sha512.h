#pragma once

#include "C:\Factory\Common\all.h"
#include "sha512\sha512.h"

typedef struct sha512_st // static member
{
	SHA512_CTX Context;
}
sha512_t;

extern uchar sha512_hash[64];
extern char sha512_hexHash[129];

sha512_t *sha512_create(void);
void sha512_release(sha512_t *i);
sha512_t *sha512_copy(sha512_t *i);
void sha512_update(sha512_t *i, autoBlock_t *text);
void sha512_makeHash(sha512_t *i);
void sha512_makeHexHash(void);
void sha512_128_makeHexHash(void);

void sha512_makeHashBlock(autoBlock_t *text);
void sha512_makeHashLine(char *line);
void sha512_makeHashFile(char *file);

void sha512_localize(void);
void sha512_unlocalize(void);

char *sha512_128Block(autoBlock_t *block);
char *sha512_128Bytes(void *bytes, uint size);
char *sha512_128Line(char *line);
char *sha512_128File(char *file);
