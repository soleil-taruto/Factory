/*
	getCryptoByte()�̃o�C�g��:

		xor(a1, a2, a3, a4)

	a1:
		Camellia(b[0], k1) +
		Camellia(b[1], k1) +
		Camellia(b[2], k1) +
		...

	a2:
		Camellia(b[2^126 + 0], k2) +
		Camellia(b[2^126 + 1], k2) +
		Camellia(b[2^126 + 2], k2) +
		...

	a3:
		Camellia(b[2^127 + 0], k3) +
		Camellia(b[2^127 + 1], k3) +
		Camellia(b[2^127 + 2], k3) +
		...

	a4:
		Camellia(b[3 * 2^126 + 0], k4) +
		Camellia(b[3 * 2^126 + 1], k4) +
		Camellia(b[3 * 2^126 + 2], k4) +
		...

	b:
		cb[0] = 0x00, 0x00, 0x00, ... 0x00, 0x00, 0x00 // 16 �o�C�g
		cb[1] = 0x01, 0x00, 0x00, ... 0x00, 0x00, 0x00
		cb[2] = 0x02, 0x00, 0x00, ... 0x00, 0x00, 0x00
		...
		cb[255] = 0xff, 0x00, 0x00, ... 0x00, 0x00, 0x00
		cb[256] = 0x00, 0x01, 0x00, ... 0x00, 0x00, 0x00
		cb[257] = 0x01, 0x01, 0x00, ... 0x00, 0x00, 0x00
		cb[258] = 0x02, 0x01, 0x00, ... 0x00, 0x00, 0x00
		...
		cb[2^128-1] = 0xff, 0xff, 0xff, ... 0xff, 0xff, 0xff
		cb[2^128+0] = 0x00, 0x00, 0x00, ... 0x00, 0x00, 0x00
		cb[2^128+1] = 0x01, 0x00, 0x00, ... 0x00, 0x00, 0x00
		cb[2^128+2] = 0x02, 0x00, 0x00, ... 0x00, 0x00, 0x00
		...

	k1:
		SHA512(s1)�̑O��256bit

	s1:
		GetCryptoSeed(, 4096, SEED_FILE) // 1�v���Z�X��1�x�����Ă΂��B

	k2:
		SHA512(s2)�̑O��256bit

	s2:
		GetCryptoSeed(, 4096, CA_SEED_FILE) // 1�v���Z�X��1�x�����Ă΂��B

	k3:
		SHA512(s3)�̑O��256bit

	s3:
		GetCryptoSeed(, 4096, CA2_SEED_FILE) // 1�v���Z�X��1�x�����Ă΂��B

	k4:
		SHA512(s4)�̑O��256bit

	s4:
		GetCryptoSeed(, 4096, CA3_SEED_FILE) // 1�v���Z�X��1�x�����Ă΂��B

	- - -

	Camellia(plain, raw-key) = ���� 256 bit �� Camellia �u���b�N�̈Í���

	2^128 * 16 �o�C�g�i�ň��ł� k1 == k2 == k3 == k4 �̂Ƃ��� 2^128 * 4 �o�C�g�j�̎����������A
	�ʓI�Ɉ�����邱�Ƃ͗L�蓾�Ȃ����낤�B

	kn == km �̂Ƃ��Aan �� am �Ɓi���� am �� an �Ɓj�����������f���o���悤�ɂȂ�܂ŁA�ŒZ�� 2^128 * 4 �o�C�g
*/

#include "CryptoRand_v2.h"

#define SEED_DIR "C:\\Factory\\tmp"
#define SEED_FILE     SEED_DIR "\\CSeed.dat"
#define CA_SEED_FILE  SEED_DIR "\\CSeedCa.dat"
#define CA2_SEED_FILE SEED_DIR "\\CSeedCa2.dat"
#define CA3_SEED_FILE SEED_DIR "\\CSeedCa3.dat"

#define SEEDSIZE 4096

static void GetCryptoSeed(uchar *seed, uint seed_size, char *seed_file)
{
	if (isFactoryDirEnabled())
	{
		autoBlock_t gab;

		mutex();

		// zantei >

		if (existFile(seed_file) && getFileSize(seed_file) != (uint64)seed_size)
		{
			cout("#########################################################\n");
			cout("## SEED_FILE SIZE ERROR -- �ǂ����ɌÂ� exe ������ŁI ##\n");
			cout("#########################################################\n");

			removeFile(seed_file);
		}

		// < zantei

		if (existFile(seed_file))
		{
			FILE *fp;
			uint index;

			fp = fileOpen(seed_file, "rb");
			fileRead(fp, gndBlockVar(seed, seed_size, gab));
			fileClose(fp);

			for (index = 0; index < seed_size; index++)
			{
				if (seed[index] < 0xff)
				{
					seed[index]++;
					break;
				}
				seed[index] = 0x00;
			}
		}
		else
		{
			getCryptoBlock_MS(seed, seed_size);
		}
		writeBinary(seed_file, gndBlockVar(seed, seed_size, gab));
		unmutex();
	}
	else
	{
		getCryptoBlock_MS(seed, seed_size);
	}
}

#define BUFFERSIZE 16 // == camellia block size

static void XorBlock(void *b1, void *b2)
{
	((uint *)b1)[0] ^= ((uint *)b2)[0];
	((uint *)b1)[1] ^= ((uint *)b2)[1];
	((uint *)b1)[2] ^= ((uint *)b2)[2];
	((uint *)b1)[3] ^= ((uint *)b2)[3];
}
static void GetCryptoBlock(uchar *buffer)
{
	static camellia_keyTable_t *cam_kt[4];
	static uchar counter[16];
	static uchar dest[16];
	uint index;

	if (!cam_kt[0]) // ������
	{
		uchar (*cam_seed)[SEEDSIZE] = (uchar (*)[SEEDSIZE])memAlloc(SEEDSIZE * 4);
		autoBlock_t gab;

		GetCryptoSeed(cam_seed[0], SEEDSIZE, SEED_FILE);
		GetCryptoSeed(cam_seed[1], SEEDSIZE, CA_SEED_FILE);
		GetCryptoSeed(cam_seed[2], SEEDSIZE, CA2_SEED_FILE);
		GetCryptoSeed(cam_seed[3], SEEDSIZE, CA3_SEED_FILE);

		sha512_localize();

		for (index = 0; index < 4; index++)
		{
			sha512_makeHashBlock(gndBlockVar(cam_seed[index], SEEDSIZE, gab));
			memcpy(cam_seed[index], sha512_hash, 32);
			cam_kt[index] = camellia_createKeyTable(gndBlockVar(cam_seed[index], 32, gab));
		}
		sha512_unlocalize();

		memFree(cam_seed);
	}
	else // �J�E���^�X�V
	{
		for (index = 0; ; index++)
//		for (index = 0; index < 16; index++)
		{
			if (counter[index] < 0xff)
			{
				counter[index]++;
				break;
			}
			counter[index] = 0x00;
		}
	}
	camellia_encrypt(cam_kt[0], counter, buffer, 1);
	counter[15] ^= 0x40;

	camellia_encrypt(cam_kt[1], counter, dest, 1);
	XorBlock(buffer, dest);
	counter[15] ^= 0xc0;

	camellia_encrypt(cam_kt[2], counter, dest, 1);
	XorBlock(buffer, dest);
	counter[15] ^= 0x40;

	camellia_encrypt(cam_kt[3], counter, dest, 1);
	XorBlock(buffer, dest);
	counter[15] ^= 0xc0;
}
uint getCryptoByte(void)
{
	static uchar buffer[BUFFERSIZE];
	static uint index = BUFFERSIZE;

	if (index == BUFFERSIZE)
	{
		GetCryptoBlock(buffer);
		index = 0;
	}
	return buffer[index++];
}
uint getCryptoRand16(void)
{
	return getCryptoByte() | getCryptoByte() << 8;
}
uint getCryptoRand24(void)
{
	return getCryptoByte() | getCryptoByte() << 8 | getCryptoByte() << 16;
}
uint getCryptoRand(void)
{
	return getCryptoByte() | getCryptoByte() << 8 | getCryptoByte() << 16 | getCryptoByte() << 24;
}
uint64 getCryptoRand64(void)
{
	return
		(uint64)getCryptoByte() <<  0 |
		(uint64)getCryptoByte() <<  8 |
		(uint64)getCryptoByte() << 16 |
		(uint64)getCryptoByte() << 24 |
		(uint64)getCryptoByte() << 32 |
		(uint64)getCryptoByte() << 40 |
		(uint64)getCryptoByte() << 48 |
		(uint64)getCryptoByte() << 56;
}
autoBlock_t *makeCryptoRandBlock(uint count)
{
	autoBlock_t *block = createBlock(count);

	while (count)
	{
		addByte(block, getCryptoByte());
		count--;
	}
	return block;
}
