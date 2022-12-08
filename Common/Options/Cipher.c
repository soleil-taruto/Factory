/*
	keyBundle:
		����
		���̑�
		���Ȃ��Ƃ���ȏ�̃L�[
		�L�[�T�C�Y�� 16, 24, 32 bytes �̉��ꂩ�A�������̃L�[�T�C�Y�͓���
		�L�[�̒[���o�C�g��F�߂Ȃ��B

		�G���[�`�F�b�N
			cphrLoadKeyBundle()
				�S�s <09AFaf> �ł���A�����͂Q�̔{���ł��邱�ƁB��t�@�C���E��s��F�߂�B

			cphrCreateKeyTableList()
				������ keySize �ȏ� && keySize �̔{���ł��邱�ƁB
*/

#include "Cipher.h"

#define MD5_SIZE 16   // ���b�Z�[�W�_�C�W�F�X�g�̃T�C�Y (MD5)
#define BLOCK_SIZE 16 // �u���b�N�Í� (camellia) �̃u���b�N�T�C�Y

#define DIV_BLOCK_SIZE_MIN (1024 * 16)         // BLOCK_SIZE �̔{���ł��邱�ƁB
#define DIV_BLOCK_SIZE_MAX (1024 * 1024 * 128) // BLOCK_SIZE �̔{���ł��邱�ƁB

autoList_t *cphrCreateKeyTableList(autoBlock_t *keyBundle, uint keySize)
{
	autoList_t *keyTableList = newList();
	uint index;
	autoBlock_t gab;

	errorCase(!keyBundle);
	errorCase(
		keySize != 16 &&
		keySize != 24 &&
		keySize != 32
		);

	errorCase(getSize(keyBundle) == 0); // ���͂P�ȏ゠�邱�ƁB
	errorCase(getSize(keyBundle) % keySize != 0);

	for (index = 0; (index + 1) * keySize <= getSize(keyBundle); index++)
	{
		addElement(
			keyTableList,
			(uint)camellia_createKeyTable(gndSubBytesVar(keyBundle, index * keySize, keySize, gab))
			);
	}
	return keyTableList;
}
void cphrReleaseKeyTableList(autoList_t *keyTableList)
{
	camellia_keyTable_t *keyTable;
	uint index;

	errorCase(!keyTableList);

	foreach (keyTableList, keyTable, index)
	{
		errorCase(!keyTable);
		camellia_releaseKeyTable(keyTable);
	}
	releaseAutoList(keyTableList);
}

autoBlock_t *cphrLoadKeyBundle(autoList_t *lines)
{
	autoBlock_t *keyBundle = newBlock();
	autoBlock_t *key;
	char *line;
	uint index;
	char *p;

	errorCase(!lines);

	foreach (lines, line, index)
	{
		errorCase(!line);
		errorCase(!lineExp("<09AFaf>", line));
		errorCase(strlen(line) % 2 != 0);

		key = makeBlockHexLine(line);
		addBytes(keyBundle, key);
		releaseAutoBlock(key);
	}
	return keyBundle;
}
autoBlock_t *cphrLoadKeyBundleFile(char *file)
{
	autoList_t *lines = readLines(file);
	autoBlock_t *keyBundle;

	keyBundle = cphrLoadKeyBundle(lines);
	releaseDim(lines, 1);
	return keyBundle;
}
static autoBlock_t *InputPassphrase2KeyBundle(char *passEx)
{
	char *passphrase;
	char *passphrase_k;
	autoBlock_t *keyBundle;

	if (*passEx)
	{
		cout("+--------------------------------------------------+\n");
		cout("| �����Ɏw�肳�ꂽ�ł��낤�p�X�t���[�Y���g�p���܂� |\n");
		cout("+--------------------------------------------------+\n");
		passphrase = strx(passEx);
	}
	else
	{
		cout("+------------------------------+\n");
		cout("| �p�X�t���[�Y����͂��ĉ����� |\n");
		cout("+------------------------------+\n");
		passphrase = coInputLineCRC();
	}
//cout("1:%s\n", passphrase); // test
	passphrase_k = kanjiPunch(passphrase, 'K');
//cout("2:%s\n", passphrase); // test
//cout("K:%s\n", passphrase_k); // test

	if (lineExp("<>[<1,,><2,09>]", passphrase_k))
	{
		char *p = strchr(passphrase, '\0') - 5;
		int xChr;
		uint xExp;
		char *xPtn;
		uint xNum;

		*p = '\0';
		p++;
		xChr = *p;
		p++;
		p[2] = '\0';
		xExp = toValue(p);

#if 1
		errorCase(!m_isRange(xExp, 20, 50));

		xPtn = repeatChar(xChr, 1024 * 1024);
		xExp -= 20;
#else // old
		errorCase(!m_isRange(xExp, 10, 40));

		xPtn = repeatChar(xChr, 1024);
		xExp -= 10;
#endif

		for (xNum = 1; xExp; xExp--)
			xNum *= 2;

		// �n�b�V������
		{
			sha512_t *i = sha512_create();
			autoBlock_t gab;
			uint count;

			sha512_update(i, gndBlockLineVar(passphrase, gab));
			gab = gndBlockLine(xPtn);

			cout("passphrase expanding...\n");
			ProgressBegin();

			for (count = 0; count < xNum; count++)
			{
				if (count % 8 == 0)
					ProgressRate((double)count / xNum);

				sha512_update(i, &gab);
			}
			ProgressEnd(0);
			cout("passphrase expanded!\n");

			sha512_makeHash(i);
			sha512_release(i);
		}
	}
	else
	{
		sha512_makeHashLine(passphrase);
	}
	sha512_makeHexHash();

	cout("(%02x) %s\n", crc8CheckBlock(sha512_hash, 64), sha512_hexHash);

	keyBundle = recreateBlock(sha512_hash, 64);
	memFree(passphrase);
	memFree(passphrase_k);
	return keyBundle;
}
/*
	fileEx == "*" �̂Ƃ��A�R���\�[��������͂��ꂽ������� SHA-512 �������Ƃ��ĕԂ��B
	fileEx == "*PASS" �̂Ƃ��APASS �� SHA-512 �������Ƃ��ĕԂ��B(PASS�͔C�ӂ̕�����)

	PASS �̏I��肪 "[CNN]" (C: ���p����, N: ����) �ł������Ƃ� PASS �� C �� 2^NN ��ǉ�����B
	NN �Ɏw��ł���͈͂� 20�`50

	�Ⴆ�� "abcdef[x30]" �� "abcdefxxxxxxxxxx...xxxxxxxxxx" �ɂȂ�B
	                               |<----- 2^30 �� ----->|
*/
autoBlock_t *cphrLoadKeyBundleFileEx(char *fileEx)
{
	if (fileEx[0] == '*')
		return InputPassphrase2KeyBundle(fileEx + 1);

	return cphrLoadKeyBundleFile(fileEx);
}

void cphrAddHash(autoBlock_t *block)
{
	autoBlock_t *hash = md5_makeHashBlock(block);

	addBytes(block, hash);
	releaseAutoBlock(hash);
}
int cphrUnaddHash(autoBlock_t *block) // ret: 0 == block �͔j�����Ă���B
{
	autoBlock_t *expectedHash;
	autoBlock_t *hash;
	int retval;

	if (getSize(block) < MD5_SIZE)
		return 0;

	expectedHash = getSubBytes(block, getSize(block) - MD5_SIZE, MD5_SIZE);
	setSize(block, getSize(block) - MD5_SIZE);
	hash = md5_makeHashBlock(block);

	retval = isSameBlock(expectedHash, hash);

	releaseAutoBlock(expectedHash);
	releaseAutoBlock(hash);

	return retval;
}

void cphrAddPadding(autoBlock_t *block) // ���Ȃ��Ƃ� block �� BLOCK_SIZE �o�C�g�ȏ�ɂȂ�B
{
	uint size = getSize(block) + 1;
	uint index;

	size %= BLOCK_SIZE;
	size = BLOCK_SIZE - size;
	size %= BLOCK_SIZE;
	size += (getCryptoByte() / BLOCK_SIZE) * BLOCK_SIZE;

	if (size < BLOCK_SIZE) size += BLOCK_SIZE; // @ 2015.1.25

	for (index = 0; index < size; index++)
	{
		addByte(block, getCryptoByte());
	}
	addByte(block, size);

	errorCase(getSize(block) < BLOCK_SIZE); // 2bs
	errorCase(getSize(block) % BLOCK_SIZE != 0); // 2bs
}
int cphrUnaddPadding(autoBlock_t *block) // ret: 0 == block �͔j�����Ă���B
{
	uint size;

	if (getSize(block) < 1)
		return 0;

	size = unaddByte(block);

	if (getSize(block) < size)
		return 0;

	setSize(block, getSize(block) - size);
	return 1;
}

#define XorBlock(block, mask) \
	((uint *)block)[0] ^= ((uint *)mask)[0]; \
	((uint *)block)[1] ^= ((uint *)mask)[1]; \
	((uint *)block)[2] ^= ((uint *)mask)[2]; \
	((uint *)block)[3] ^= ((uint *)mask)[3]

uchar *cphrEncryptBlock(uchar *block, autoList_t *keyTableList, uchar *iv)
{
	camellia_keyTable_t *keyTable;
	uint index;

//	errorCase((uint)block & 3 || (uint)iv & 3);

	XorBlock(block, iv);

	foreach (keyTableList, keyTable, index)
	{
		camellia_encrypt(keyTable, block, block, 1);
	}
	return block;
}
uchar *cphrDecryptBlock(uchar *block, autoList_t *keyTableList, uchar *iv)
{
	static uchar niv[BLOCK_SIZE * 2];
	static uint nii;
	uint count;

//	errorCase((uint)block & 3 || (uint)iv & 3);

	nii = BLOCK_SIZE - nii;
	memcpy(niv + nii, block, BLOCK_SIZE);

	for (count = getCount(keyTableList); count; count--)
	{
		camellia_encrypt(
			(camellia_keyTable_t *)getElement(keyTableList, count - 1),
			block,
			block,
			0
			);
	}
	XorBlock(block, iv);
	return niv + nii;
}
void cphrEncryptor(autoBlock_t *block, autoList_t *keyTableList, autoBlock_t *iv, int doEncrypt) // iv ���X�V����B
{
	uchar *biv;
	uint index;

	errorCase(!block);
	errorCase(!keyTableList);
	errorCase(!iv);

	errorCase(getSize(block) % BLOCK_SIZE != 0);
	errorCase(getCount(keyTableList) == 0);
	errorCase(getSize(iv) != BLOCK_SIZE);

	biv = (uchar *)directGetBuffer(iv);

	for (index = 0; index < getSize(block); index += BLOCK_SIZE)
	{
		biv = (doEncrypt ? cphrEncryptBlock : cphrDecryptBlock)((uchar *)directGetBuffer(block) + index, keyTableList, biv);
	}
	memcpy(directGetBuffer(iv), biv, BLOCK_SIZE);
}

void cphrEncryptorBlock(autoBlock_t *block, autoList_t *keyTableList)
{
	autoBlock_t *iv = makeCryptoRandBlock(BLOCK_SIZE);
	autoBlock_t *jv;

	jv = copyAutoBlock(iv);

	errorCase(!block);
//	errorCase(!keyTableList);

	cphrAddHash(block);
	cphrAddPadding(block);
	cphrEncryptor(block, keyTableList, iv, 1);
	addBytes(block, jv);

	releaseAutoBlock(iv);
	releaseAutoBlock(jv);
}
int cphrDecryptorBlock(autoBlock_t *block, autoList_t *keyTableList) // ret: 0 == block �͔j�����Ă���B
{
	autoBlock_t *iv;
	int retval = 0;

	errorCase(!block);
//	errorCase(!keyTableList);

	if (getSize(block) < BLOCK_SIZE) // iv �Ȃ�
		return 0;

	iv = getSubBytes(block, getSize(block) - BLOCK_SIZE, BLOCK_SIZE);
	setSize(block, getSize(block) - BLOCK_SIZE);

	cphrEncryptor(block, keyTableList, iv, 0);

	if (!cphrUnaddPadding(block)) goto endfunc;
	if (!cphrUnaddHash(block))    goto endfunc;

	retval = 1;

endfunc:
	releaseAutoBlock(iv);
	return retval;
}

static uint G_DivBlockSize;

static uint GetDivBlockSize(void)
{
	static uint lastTime;
	uint currTime = now();
	uint size = G_DivBlockSize;

	if (lastTime == currTime)
		size += size / 8;
	else if (lastTime + 2 <= currTime)
		size -= size / 8;

	lastTime = currTime;

	size /= BLOCK_SIZE;
	size *= BLOCK_SIZE;
	m_range(size, DIV_BLOCK_SIZE_MIN, DIV_BLOCK_SIZE_MAX);

	G_DivBlockSize = size;
	return size;
}
/*
	srcFile, destFile:
		�����ɃX�g���[�����J���̂ŁA�قȂ�t�@�C���ł��邱�ƁB

	interlude:
		���ɕK�v�Ȃ���� noop() ���w�肷�邱�ƁB
*/
void cphrEncryptorFile(char *srcFile, char *destFile, autoList_t *keyTableList, void (*interlude)(void))
{
	autoBlock_t *firstiv = makeCryptoRandBlock(BLOCK_SIZE);
	autoBlock_t *iv;
	FILE *rfp;
	FILE *wfp;
	md5_t *md5 = md5_create();

	errorCase(!srcFile);
	errorCase(!destFile);
	errorCase(!keyTableList);
	errorCase(!interlude);

	iv = copyAutoBlock(firstiv);

	rfp = fileOpen(srcFile, "rb");
	wfp = fileOpen(destFile, "wb");

	G_DivBlockSize = 0;

	for (; ; )
	{
		uint divBlockSize = GetDivBlockSize();
		autoBlock_t *block;
		autoBlock_t *pab;
		int rfpend = 0;

		block = readBinaryBlock(rfp, divBlockSize);
		md5_update(md5, block);

		if (getSize(block) < divBlockSize)
		{
			addBytes(block, pab = md5_makeHash(md5));
			releaseAutoBlock(pab);
			cphrAddPadding(block);
			rfpend = 1;
		}
		cphrEncryptor(block, keyTableList, iv, 1);
		writeBinaryBlock(wfp, block);
		releaseAutoBlock(block);

		if (rfpend)
			break;

		interlude();
	}
	writeBinaryBlock(wfp, firstiv);

	fileClose(rfp);
	fileClose(wfp);

	releaseAutoBlock(firstiv);
	releaseAutoBlock(iv);
	md5_release(md5);
}
int cphrDecryptorFile(char *srcFile, char *destFile, autoList_t *keyTableList, void (*interlude)(void)) // ret: 0 == srcFile �̓��e���j�����Ă���B
{
	uint64 srcFileSize;
	uint64 srcIndex = 0ui64;
	FILE *rfp;
	FILE *wfp;
	autoBlock_t *iv = NULL;
	autoBlock_t *block = NULL;
	autoBlock_t *hash = NULL;
	autoBlock_t *expectedHash = NULL;
	md5_t *md5 = NULL;
	int retval = 0;

	errorCase(!srcFile);
	errorCase(!destFile);
	errorCase(!keyTableList);
	errorCase(!interlude);

	srcFileSize = getFileSize(srcFile);

	rfp = fileOpen(srcFile, "rb");
	wfp = fileOpen(destFile, "wb");

	if (srcFileSize < BLOCK_SIZE)
		goto endfunc;

	if (_fseeki64(rfp, srcFileSize - BLOCK_SIZE, SEEK_SET) != 0) // ? �V�[�N���s
		error();

	iv = readBinaryBlock(rfp, BLOCK_SIZE);

	if (_fseeki64(rfp, 0i64, SEEK_SET) != 0) // ? �V�[�N���s
		error();

	md5 = md5_create();
	G_DivBlockSize = 0;

	for (; ; )
	{
		uint divBlockSize = GetDivBlockSize();
		int endblock;

		// �Ō�� block ���Z������Ƃ܂����̂ŁA������x�Z���[���͈ꏏ�ɓǂݍ���ł��܂��B
		if (srcFileSize - srcIndex < divBlockSize + divBlockSize / 8)
		{
			uint remSize = srcFileSize - srcIndex;

			// iv �����B
			errorCase(remSize < BLOCK_SIZE); // �ŏ��ɓǂ߂��͂��B
			remSize -= BLOCK_SIZE;

			block = readBinaryBlock(rfp, remSize);
			endblock = 1;
		}
		else
		{
			block = readBinaryBlock(rfp, divBlockSize);
			endblock = 0;
		}
		cphrEncryptor(block, keyTableList, iv, 0);

		if (endblock)
		{
			if (!cphrUnaddPadding(block))
				goto endfunc;

			if (getSize(block) < MD5_SIZE)
				goto endfunc;

			expectedHash = getSubBytes(block, getSize(block) - MD5_SIZE, MD5_SIZE);
			setSize(block, getSize(block) - MD5_SIZE);
		}
		md5_update(md5, block);

		if (endblock)
			if (!isSameBlock(expectedHash, hash = md5_makeHash(md5))) // ? �n�b�V���s��v
				goto endfunc;

		writeBinaryBlock(wfp, block);

		if (endblock)
			break;

		srcIndex += getSize(block);
		releaseAutoBlock(block);
		interlude();
	}
	retval = 1;

endfunc:
	fileClose(rfp);
	fileClose(wfp);

	if (iv)           releaseAutoBlock(iv);
	if (block)        releaseAutoBlock(block);
	if (hash)         releaseAutoBlock(hash);
	if (expectedHash) releaseAutoBlock(expectedHash);
	if (md5)          md5_release(md5);

	return retval;
}
