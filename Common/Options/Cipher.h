#pragma once

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\CryptoRand.h"
#include "C:\Factory\Common\Options\Progress.h"
#include "C:\Factory\OpenSource\camellia.h"
#include "C:\Factory\OpenSource\md5.h"

// -- InputPassphrase2KeyBundle() —p ‚±‚±‚©‚ç

#include "C:\Factory\OpenSource\sha512.h"
#include "C:\Factory\Common\Options\crc.h"

// -- InputPassphrase2KeyBundle() —p ‚±‚±‚Ü‚Å

autoList_t *cphrCreateKeyTableList(autoBlock_t *keyBundle, uint keySize);
void cphrReleaseKeyTableList(autoList_t *keyTableList);
autoBlock_t *cphrLoadKeyBundle(autoList_t *lines);
autoBlock_t *cphrLoadKeyBundleFile(char *file);
autoBlock_t *cphrLoadKeyBundleFileEx(char *fileEx);

void cphrAddHash(autoBlock_t *block);
int cphrUnaddHash(autoBlock_t *block);
void cphrAddPadding(autoBlock_t *block);
int cphrUnaddPadding(autoBlock_t *block);

uchar *cphrEncryptBlock(uchar *block, autoList_t *keyTableList, uchar *iv);
uchar *cphrDecryptBlock(uchar *block, autoList_t *keyTableList, uchar *iv);
void cphrEncryptor(autoBlock_t *block, autoList_t *keyTableList, autoBlock_t *iv, int doEncrypt);

void cphrEncryptorBlock(autoBlock_t *block, autoList_t *keyTableList);
int cphrDecryptorBlock(autoBlock_t *block, autoList_t *keyTableList);
void cphrEncryptorFile(char *srcFile, char *destFile, autoList_t *keyTableList, void (*interlude)(void));
int cphrDecryptorFile(char *srcFile, char *destFile, autoList_t *keyTableList, void (*interlude)(void));
