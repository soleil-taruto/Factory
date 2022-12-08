#include "camellia.h"

camellia_keyTable_t *camellia_createKeyTable(autoBlock_t *rawKey)
{
	camellia_keyTable_t *i = (camellia_keyTable_t *)memAlloc(sizeof(camellia_keyTable_t));

	errorCase(!rawKey);

	switch (getSize(rawKey))
	{
	case 16: // 128bits
	case 24: // 192bits
	case 32: // 256bits
		break;

	default:
		error();
	}

	i->KeyBitLength = getSize(rawKey) * 8;
	camellia_Ekeygen(i->KeyBitLength, directGetBuffer(rawKey), i->KeyTable);

	return i;
}
void camellia_releaseKeyTable(camellia_keyTable_t *i)
{
	errorCase(!i);
	memFree(i);
}
void camellia_encrypt(camellia_keyTable_t *i, void *in, void *out, int doEncrypt) // in == out ‰Â
{
	errorCase(!i);
	errorCase(!in);
	errorCase(!out);

	( doEncrypt ? camellia_EncryptBlock : camellia_DecryptBlock )(i->KeyBitLength, in, i->KeyTable, out);
}
void camellia_encryptBlock(camellia_keyTable_t *i, autoBlock_t *in, autoBlock_t *out, int doEncrypt)
{
	errorCase(!i);
	errorCase(!in);
	errorCase(!out);

	errorCase(getSize(in) != 16);
	errorCase(getSize(out) != 16);

	camellia_encrypt(i, directGetBuffer(in), directGetBuffer(out), doEncrypt);
}

void camellia_ecb(camellia_keyTable_t *i, autoBlock_t *in, autoBlock_t *out, int doEncrypt) // in == out ‰Â
{
	uchar *bin;
	uchar *bout;
	uint index;

	errorCase(!i);
	errorCase(!in);
	errorCase(!out);

	errorCase(getSize(in) % 16 != 0);
//	errorCase(getSize(out) % 16 != 0);
	errorCase(getSize(in) != getSize(out));

	bin = (uchar *)directGetBuffer(in);
	bout = (uchar *)directGetBuffer(out);

	for (index = 0; index < getSize(in); index += 16)
	{
		camellia_encrypt(i, bin + index, bout + index, doEncrypt);
	}
}

static void XorBlock(uchar *in1, uchar *in2, uchar *out)
{
#if 1
	((uint *)out)[0] = ((uint *)in1)[0] ^ ((uint *)in2)[0];
	((uint *)out)[1] = ((uint *)in1)[1] ^ ((uint *)in2)[1];
	((uint *)out)[2] = ((uint *)in1)[2] ^ ((uint *)in2)[2];
	((uint *)out)[3] = ((uint *)in1)[3] ^ ((uint *)in2)[3];
#else
	uint index;

	for (index = 0; index < 16; index++)
	{
		out[index] = in1[index] ^ in2[index];
	}
#endif
}
void camellia_cbc(camellia_keyTable_t *i, autoBlock_t *iv, autoBlock_t *in, autoBlock_t *out, int doEncrypt) // in == out ‰Â
{
	uchar *biv;
	uchar *bin;
	uchar *bout;
	uint index;

	errorCase(!i);
	errorCase(!iv);
	errorCase(!in);
	errorCase(!out);

	errorCase(getSize(iv) != 16);
	errorCase(getSize(in) % 16 != 0);
//	errorCase(getSize(out) % 16 != 0);
	errorCase(getSize(in) != getSize(out));

	if (!getSize(in))
		return;

	biv = (uchar *)directGetBuffer(iv);
	bin = (uchar *)directGetBuffer(in);
	bout = (uchar *)directGetBuffer(out);

	if (doEncrypt)
	{
		XorBlock(biv, bin, bout);
		camellia_encrypt(i, bout, bout, 1);

		for (index = 16; index < getSize(in); index += 16)
		{
			XorBlock(bout + index - 16, bin + index, bout + index);
			camellia_encrypt(i, bout + index, bout + index, 1);
		}
	}
	else
	{
		for (index = getSize(in) - 16; index; index -= 16)
		{
			camellia_encrypt(i, bin + index, bout + index, 0);
			XorBlock(bin + index - 16, bout + index, bout + index);
		}
		camellia_encrypt(i, bin + index, bout + index, 0);
		XorBlock(biv, bout, bout);
	}
}
void camellia_cbcRing(camellia_keyTable_t *i, autoBlock_t *in, autoBlock_t *out, int doEncrypt) // in == out ‰Â
{
	autoBlock_t wrkiv;

	errorCase(!i);
	errorCase(!in);
	errorCase(!out);

	errorCase(getSize(in) < 32);
	errorCase(getSize(in) % 16 != 0);
//	errorCase(getSize(out) % 16 != 0);
	errorCase(getSize(in) != getSize(out));

	camellia_cbc(i, gndSubBytesVar(doEncrypt ? in : out, getSize(in) - 16, 16, wrkiv), in, out, doEncrypt);
}

static void CamelliaCtr(camellia_keyTable_t *i, autoBlock_t *iv, autoBlock_t *in, autoBlock_t *out, int ofbmode)
{
	uchar ctr[16];
	uchar eb[16];
	uchar *bin;
	uchar *bout;
	uint index;
	uint ctridx;

	errorCase(!i);
	errorCase(!iv);
	errorCase(!in);
	errorCase(!out);

	errorCase(getSize(iv) != 16);
	errorCase(getSize(in) != getSize(out));

	memcpy(ctr, directGetBuffer(iv), 16);
	memcpy(eb, ctr, 16);
	bin = (uchar *)directGetBuffer(in);
	bout = (uchar *)directGetBuffer(out);

	for (index = 0; index < getSize(in); index++)
	{
		if ((index & 0x0f) == 0)
		{
			if (!ofbmode)
			{
				camellia_encrypt(i, ctr, eb, 1);

				for (ctridx = 0; ctridx < 16; ctridx++)
					if (ctr[ctridx] = (ctr[ctridx] + 1) & 0xff)
						break;
			}
			else
			{
				camellia_encrypt(i, eb, eb, 1);
			}
		}
		bout[index] = bin[index] ^ eb[index & 0x0f];
	}
}
void camellia_ctr(camellia_keyTable_t *i, autoBlock_t *iv, autoBlock_t *in, autoBlock_t *out) // in == out ‰Â, iv: •ÏX‚·‚é
{
	CamelliaCtr(i, iv, in, out, 0);
}
void camellia_ofb(camellia_keyTable_t *i, autoBlock_t *iv, autoBlock_t *in, autoBlock_t *out) // in == out ‰Â
{
	CamelliaCtr(i, iv, in, out, 1);
}

static void CamelliaCfbBlock(camellia_keyTable_t *i, uchar iv[16], uchar in[16], uchar out[16], uint index, uint size)
{
	uchar eb[16];
	uint bndx;

	camellia_encrypt(i, index ? iv : in + index - 16, eb, 1);

	for (bndx = 0; bndx < 16 && index + bndx < size; bndx++)
	{
		out[bndx] = in[bndx] ^ eb[bndx];
	}
}
void camellia_cfb(camellia_keyTable_t *i, autoBlock_t *iv, autoBlock_t *in, autoBlock_t *out, int doEncrypt) // in == out ‰Â
{
	uchar *biv;
	uchar *bin;
	uchar *bout;
	uint index;

	errorCase(!i);
	errorCase(!iv);
	errorCase(!in);
	errorCase(!out);

	errorCase(getSize(iv) != 16);
	errorCase(getSize(in) != getSize(out));

	biv = (uchar *)directGetBuffer(iv);
	bin = (uchar *)directGetBuffer(in);
	bout = (uchar *)directGetBuffer(out);

	if (doEncrypt)
	{
		for (index = 0; index < getSize(in); index += 16)
		{
			CamelliaCfbBlock(i, biv, bin, bout, index, getSize(in));
		}
	}
	else
	{
		for (index = getSize(in) & ~15; ; index -= 16)
		{
			CamelliaCfbBlock(i, biv, bin, bout, index, getSize(in));

			if (!index)
				break;
		}
	}
}
