/*
	getCryptoByte()のバイト列:

		xor(a, ca, ca2)

	a:
		SHA512(b[0]) + SHA512(b[1]) + SHA512(b[2]) + ...

	b:
		b[0] = s
		b[1] = s + c[0]
		b[2] = s + c[1]
		b[3] = s + c[2]
		...

	s:
		GetCryptoSeed(, 4096, SEED_FILE) // 1プロセス中1度だけ呼ばれる。

	c:
		c[0] = 0x00
		c[1] = 0x01
		c[2] = 0x02
		...
		c[255] = 0xff
		c[256] = 0x00, 0x00
		c[257] = 0x01, 0x00
		c[258] = 0x02, 0x00
		...
		c[511] = 0xff, 0x00
		c[512] = 0x00, 0x01
		c[513] = 0x01, 0x01
		c[514] = 0x02, 0x01
		...
		c[65791] = 0xff, 0xff
		c[65792] = 0x00, 0x00, 0x00
		c[65793] = 0x01, 0x00, 0x00
		c[65794] = 0x02, 0x00, 0x00
		...

	ca:
		Camellia(cb[0], ck) + Camellia(cb[1], ck) + Camellia(cb[2], ck) + ...

	cb:
		cb[0] = 0x00, 0x00, 0x00, ... 0x00, 0x00, 0x00 // 16 バイト
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

	ck:
		SHA512(cs)の前半256bit

	cs:
		GetCryptoSeed(, 4096, CA_SEED_FILE) // 1プロセス中1度だけ呼ばれる。

	ca2:
		Camellia(cb2[0], ck2) + Camellia(cb2[1], ck2) + Camellia(cb2[2], ck2) + ...

	cb2:
		cb[0] = 0x00, 0x00, 0x00, ... 0x00, 0x00, 0x80 // 16 バイト
		cb[1] = 0x01, 0x00, 0x00, ... 0x00, 0x00, 0x80
		cb[2] = 0x02, 0x00, 0x00, ... 0x00, 0x00, 0x80
		...
		cb[255] = 0xff, 0x00, 0x00, ... 0x00, 0x00, 0x80
		cb[256] = 0x00, 0x01, 0x00, ... 0x00, 0x00, 0x80
		cb[257] = 0x01, 0x01, 0x00, ... 0x00, 0x00, 0x80
		cb[258] = 0x02, 0x01, 0x00, ... 0x00, 0x00, 0x80
		...
		cb[2^127-1] = 0xff, 0xff, 0xff, ... 0xff, 0xff, 0xff
		cb[2^127+0] = 0x00, 0x00, 0x00, ... 0x00, 0x00, 0x00
		cb[2^127+1] = 0x01, 0x00, 0x00, ... 0x00, 0x00, 0x00
		cb[2^127+2] = 0x02, 0x00, 0x00, ... 0x00, 0x00, 0x00
		...

	ck2:
		SHA512(cs2)の前半256bit

	cs2:
		GetCryptoSeed(, 4096, CA2_SEED_FILE) // 1プロセス中1度だけ呼ばれる。

	- - -

	Camellia(plain, raw-key) = 鍵長 256 bit の Camellia ブロックの暗号化
*/

#include "CryptoRand_v1.h"

#define SEED_DIR "C:\\Factory\\tmp"
#define SEED_FILE     SEED_DIR "\\CSeed_v1.dat"
#define CA_SEED_FILE  SEED_DIR "\\CSeedCa_v1.dat"
#define CA2_SEED_FILE SEED_DIR "\\CSeedCa2_v1.dat"

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
			cout("## SEED_FILE SIZE ERROR -- どっかに古い exe があるで！ ##\n");
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

#define BUFFERSIZE 64 // == sha512 hash size

static void GetCryptoBlock(uchar *buffer)
{
	static sha512_t *ctx;

	sha512_localize();

	if (!ctx)
	{
		uchar seed[SEEDSIZE];
		autoBlock_t gab;

		GetCryptoSeed(seed, SEEDSIZE, SEED_FILE);

		ctx = sha512_create();
		sha512_update(ctx, gndBlockVar(seed, SEEDSIZE, gab));
		sha512_makeHash(ctx);
	}
	else
	{
		static autoBlock_t *tremor;
		sha512_t *ictx = sha512_copy(ctx);

		if (!tremor)
			tremor = newBlock();

		// tremor更新
		{
			uint index;

			for (index = 0; index < getSize(tremor); index++)
			{
				uint byteVal = getByte(tremor, index);

				if (byteVal < 0xff)
				{
					setByte(tremor, index, byteVal + 1);
					break;
				}
				setByte(tremor, index, 0x00);
			}
			if (index == getSize(tremor))
			{
				addByte(tremor, 0x00);
			}
		}

		sha512_update(ictx, tremor);
		sha512_makeHash(ictx);
		sha512_release(ictx);
	}
	memcpy(buffer, sha512_hash, BUFFERSIZE);
	sha512_unlocalize();
}
static void XorBlock(void *b1, void *b2)
{
	((uint *)b1)[0] ^= ((uint *)b2)[0];
	((uint *)b1)[1] ^= ((uint *)b2)[1];
	((uint *)b1)[2] ^= ((uint *)b2)[2];
	((uint *)b1)[3] ^= ((uint *)b2)[3];
}
static uchar *Ca_GetCryptoBlock(void)
{
	static camellia_keyTable_t *cam_kt[2];
	static uchar counter[16];
	static uchar dest[2][16];

	if (!cam_kt[0])
	{
		uchar cam_seed[2][SEEDSIZE];
		autoBlock_t gab;

		GetCryptoSeed(cam_seed[0], SEEDSIZE, CA_SEED_FILE);
		GetCryptoSeed(cam_seed[1], SEEDSIZE, CA2_SEED_FILE);

		sha512_localize();

		sha512_makeHashBlock(gndBlockVar(cam_seed[0], SEEDSIZE, gab));
		memcpy(cam_seed[0], sha512_hash, 32);
		sha512_makeHashBlock(gndBlockVar(cam_seed[1], SEEDSIZE, gab));
		memcpy(cam_seed[1], sha512_hash, 32);

		sha512_unlocalize();

		cam_kt[0] = camellia_createKeyTable(gndBlockVar(cam_seed[0], 32, gab));
		cam_kt[1] = camellia_createKeyTable(gndBlockVar(cam_seed[1], 32, gab));
	}
	camellia_encrypt(cam_kt[0], counter, dest[0], 1);
	counter[15] ^= 0x80;
	camellia_encrypt(cam_kt[1], counter, dest[1], 1);
	counter[15] ^= 0x80;

	// counter更新
	{
		uint index;

		for (index = 0; index < 16; index++)
		{
			if (counter[index] < 0xff)
			{
				counter[index]++;
				break;
			}
			counter[index] = 0x00;
		}
	}

	XorBlock(dest[0], dest[1]);
	return dest[0];
}
static void Ca_XorCryptoBlock(uchar buffer[64])
{
	XorBlock(buffer +  0, Ca_GetCryptoBlock());
	XorBlock(buffer + 16, Ca_GetCryptoBlock());
	XorBlock(buffer + 32, Ca_GetCryptoBlock());
	XorBlock(buffer + 48, Ca_GetCryptoBlock());
}
uint getCryptoByte(void)
{
	static uchar buffer[BUFFERSIZE];
	static uint index = BUFFERSIZE;

	if (index == BUFFERSIZE)
	{
		GetCryptoBlock(buffer);
		Ca_XorCryptoBlock(buffer);
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
