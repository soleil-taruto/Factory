#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\RingCipher2.h"
#include "C:\Factory\DevTools\libs\RandData.h"

static void Test01(uint lmtSize)
{
	autoBlock_t *p = MakeRandBinaryBlock(mt19937_rnd(lmtSize));
	autoBlock_t *e;
	autoBlock_t *d;
	autoBlock_t *fe;
	autoBlock_t *fd;
	autoBlock_t *e_fd;
	autoBlock_t *fe_d;
	autoList_t *ktl;
	autoBlock_t gab;
	char *file = makeTempFile(NULL);

	cout("p_size: %u\n", getSize(p));

	ktl = newList();
	addElement(ktl, (uint)camellia_createKeyTable(gndBlockLineVar("0123456789abcdef", gab)));
	addElement(ktl, (uint)camellia_createKeyTable(gndBlockLineVar("0123456789ABCDEF", gab)));
	addElement(ktl, (uint)camellia_createKeyTable(gndBlockLineVar("1234abcd1234abcd", gab)));

	// ---- �������u���b�N�� ----

	LOGPOS();

	e = copyAutoBlock(p);
	rngcphrEncrypt(e, ktl);

	LOGPOS();

	d = copyAutoBlock(e);
	errorCase(!rngcphrDecrypt(d, ktl)); // ? �������s

	// ---- �t�@�C���� ----

	LOGPOS();

	writeBinary(file, p);
	rngcphrEncryptFile(file, ktl);
	fe = readBinary(file);

	LOGPOS();

	writeBinary(file, fe);
	errorCase(!rngcphrDecryptFile(file, ktl)); // ? �������s
	fd = readBinary(file);

	// ---- �������ĕ��� ----

	LOGPOS();

	writeBinary(file, e);
	errorCase(!rngcphrDecryptFile(file, ktl)); // ? �������s
	e_fd = readBinary(file);

	LOGPOS();

	fe_d = copyAutoBlock(fe);
	errorCase(!rngcphrDecrypt(fe_d, ktl)); // ? �������s

	// ----

	LOGPOS();

	errorCase(!isSameBlock(p, d));
//	errorCase(!isSameBlock(e, fe)); // PADDING, RANDPART ������̂ňႤ�B
	errorCase(!isSameBlock(d, fd));
	errorCase(!isSameBlock(d, e_fd));
	errorCase(!isSameBlock(d, fe_d));

	releaseAutoBlock(p);
	releaseAutoBlock(e);
	releaseAutoBlock(d);
	releaseAutoBlock(fe);
	releaseAutoBlock(fd);
	releaseAutoBlock(e_fd);
	releaseAutoBlock(fe_d);

	camellia_releaseKeyTable((camellia_keyTable_t *)getElement(ktl, 0));
	camellia_releaseKeyTable((camellia_keyTable_t *)getElement(ktl, 1));
	camellia_releaseKeyTable((camellia_keyTable_t *)getElement(ktl, 2));
	releaseAutoList(ktl);

	removeFile(file);
	memFree(file);

	cout("OK\n");
}
int main(int argc, char **argv)
{
	uint testcnt;

	mt19937_init();

	for (testcnt = 0; testcnt < 100; testcnt++) Test01(10);
	for (testcnt = 0; testcnt < 100; testcnt++) Test01(100);
	for (testcnt = 0; testcnt < 100; testcnt++) Test01(1000);
	for (testcnt = 0; testcnt <  30; testcnt++) Test01(10000);
	for (testcnt = 0; testcnt <  30; testcnt++) Test01(100000);
	for (testcnt = 0; testcnt <  30; testcnt++) Test01(1000000);
}
