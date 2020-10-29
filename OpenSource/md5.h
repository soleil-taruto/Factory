#pragma once

#include "C:\Factory\Common\all.h"
#include "md5\md5.h"

extern void (*md5_interrupt)(void);

typedef struct md5_st // static member
{
	md5_CTX Context;
}
md5_t;

md5_t *md5_create(void);
void md5_release(md5_t *i);
void md5_update(md5_t *i, autoBlock_t *text);
autoBlock_t *md5_makeHash(md5_t *i);

autoBlock_t *md5_makeHashBlock(autoBlock_t *text);
autoBlock_t *md5_makeHashLine(char *line);
autoBlock_t *md5_makeHashFileBuffSize(char *file, uint buffSize);
autoBlock_t *md5_makeHashFile(char *file);

// c_
autoBlock_t *c_md5_makeHash(md5_t *i);
autoBlock_t *c_md5_makeHashBlock(autoBlock_t *text);
autoBlock_t *c_md5_makeHashLine(char *line);
autoBlock_t *c_md5_makeHashFileBuffSize(char *file, uint buffSize);
autoBlock_t *c_md5_makeHashFile(char *file);

// _x
autoBlock_t *md5_makeHashBlock_x(autoBlock_t *text);
autoBlock_t *md5_makeHashLine_x(char *line);
autoBlock_t *md5_makeHashFileBuffSize_x(char *file, uint buffSize);
autoBlock_t *md5_makeHashFile_x(char *file);

// c_x
autoBlock_t *c_md5_makeHashBlock_x(autoBlock_t *text);
autoBlock_t *c_md5_makeHashLine_x(char *line);
autoBlock_t *c_md5_makeHashFileBuffSize_x(char *file, uint buffSize);
autoBlock_t *c_md5_makeHashFile_x(char *file);

// makeHex*
char *md5_makeHexHash(md5_t *i);
char *md5_makeHexHashBlock(autoBlock_t *text);
char *md5_makeHexHashLine(char *line);
char *md5_makeHexHashFileBuffSize(char *file, uint buffSize);
char *md5_makeHexHashFile(char *file);

// c_makeHex*
char *c_md5_makeHexHash(md5_t *i);
char *c_md5_makeHexHashBlock(autoBlock_t *text);
char *c_md5_makeHexHashLine(char *line);
char *c_md5_makeHexHashFileBuffSize(char *file, uint buffSize);
char *c_md5_makeHexHashFile(char *file);

// makeHex*_x
char *md5_makeHexHashBlock_x(autoBlock_t *text);
char *md5_makeHexHashLine_x(char *line);
char *md5_makeHexHashFileBuffSize_x(char *file, uint buffSize);
char *md5_makeHexHashFile_x(char *file);

// c_makeHex*_x
char *c_md5_makeHexHashBlock_x(autoBlock_t *text);
char *c_md5_makeHexHashLine_x(char *line);
char *c_md5_makeHexHashFileBuffSize_x(char *file, uint buffSize);
char *c_md5_makeHexHashFile_x(char *file);
