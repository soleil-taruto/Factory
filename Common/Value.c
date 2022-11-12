#include "all.h"

char *decimal = "0123456789";
char *binadecimal = "01";
char *octodecimal = "01234567";
char *hexadecimal = "0123456789abcdef";

uint64 toValue64Digits(char *line, char *digits)
{
	uint radix = strlen(digits);
	char *p;
	uint64 value = 0;

	for (p = line; *p; p++)
	{
		int d = *p;
		char *dp;

		dp = strchr(digits, d);

		if (!dp)
			dp = strchr(digits, m_islower(d) ? m_nctoupper(d) : m_tolower(d));

		if (dp)
		{
			value *= radix;
			value += (uint)dp - (uint)digits;
		}
		/*
		else if (d == '.') // ? è¨êîì_
		{
			break;
		}
		*/
	}
	return value;
}
uint toValueDigits(char *line, char *digits)
{
	return (uint)toValue64Digits(line, digits);
}
uint64 toValue64(char *line)
{
	return toValue64Digits(line, decimal);
}
uint toValue(char *line)
{
	return toValueDigits(line, decimal);
}

sint64 toInt64Digits(char *line, char *digits)
{
	uint64 value = toValue64Digits(line, digits);

	m_minim(value, SINT64MAX);

	if (strchr(line, '-'))
		return -(sint64)value;

	return (sint64)value;
}
sint toIntDigits(char *line, char *digits)
{
	return (sint)toInt64Digits(line, digits);
}
sint64 toInt64(char *line)
{
	return toInt64Digits(line, decimal);
}
sint toInt(char *line)
{
	return toIntDigits(line, decimal);
}

char *toLineValue64Digits(uint64 value, char *digits)
{
	char *buff = (char *)memAlloc(65);
	char *p;
	uint radix = strlen(digits);

	errorCase(radix < 2);

	for (p = buff; 0ui64 < value; p++)
	{
		*p = digits[(uint)(value % (uint64)radix)];
		value /= (uint64)radix;
	}
	if (p == buff)
		*p++ = digits[0];

	*p = '\0';
	reverseLine(buff);
	return buff;
}
char *toLineValueDigits(uint value, char *digits)
{
	return toLineValue64Digits((uint64)value, digits);
}
char *toLineValue64(uint64 value)
{
	return toLineValue64Digits(value, decimal);
}
char *toLineValue(uint value)
{
	return toLineValueDigits(value, decimal);
}

uint iSqrt64(uint64 value)
{
	uint ret = 0;
	uint nBit;

	for (nBit = 1u << 31; nBit; nBit >>= 1)
	{
		uint tRet = ret | nBit;

		if ((uint64)tRet * tRet <= value)
			ret = tRet;
	}
	return ret;
}
uint iSqrt(uint value)
{
	return iSqrt64((uint64)value);
}
uint iRoot64(uint64 value, uint extend)
{
	uint ret = 0;
	uint nBit;

	errorCase(!extend);

	for (nBit = 1u << 31; nBit; nBit >>= 1)
	{
		uint tRet = ret | nBit;
		uint64 p;
		uint e;

		p = tRet;

		for (e = extend - 1; e; e--)
		{
			if (UINT64MAX / p < tRet)
			{
				goto overflow;
			}
			p *= tRet;
		}
		if (p <= value)
		{
			ret = tRet;
		}
	overflow:;
	}
	return ret;
}
uint iRoot(uint value, uint extend)
{
	return iRoot64((uint64)value, extend);
}

double dPow(double value, uint extend)
{
#if 1
	double answer;

	if (!extend)
		return 1.0;

	if (extend == 1)
		return value;

	answer = dPow(value, extend / 2);
	answer *= answer;

	if (extend & 1)
		answer *= value;

	return answer;
#else // old
	double answer;

	if (!extend)
		return 1.0;

	answer = value;

	while (--extend)
		answer *= value;

	return answer;
#endif
}

/*
	Little Endian
*/
void valueToBlock(void *block, uint value)
{
	((uchar *)block)[0] = value >>  0 & 0xff;
	((uchar *)block)[1] = value >>  8 & 0xff;
	((uchar *)block)[2] = value >> 16 & 0xff;
	((uchar *)block)[3] = value >> 24 & 0xff;
}
uint blockToValue(void *block)
{
	return
		((uchar *)block)[0] <<  0 |
		((uchar *)block)[1] <<  8 |
		((uchar *)block)[2] << 16 |
		((uchar *)block)[3] << 24;
}
void value64ToBlock(void *block, uint64 value)
{
	valueToBlock(block, (uint)value);
	valueToBlock((uchar *)block + 4, (uint)(value >> 32));
}
uint64 blockToValue64(void *block)
{
	return
		(uint64)blockToValue(block) |
		(uint64)blockToValue((uchar *)block + 4) << 32;
}

sint64 d2i64(double value)
{
	return (sint64)(value < 0.0 ? value - 0.5 : value + 0.5);
}
sint d2i(double value)
{
	return (sint)d2i64(value);
}

sint divRndOff(sint numer, sint denom)
{
	return divRndOffRate(numer, denom, 1, 2);
}
sint divRndOffRate(sint numer, sint denom, sint rndOffRateNumer, sint rndOffRateDenom)
{
	return (sint)divRndOff64Rate((sint64)numer, (sint64)denom, rndOffRateNumer, rndOffRateDenom);
}
sint64 divRndOff64(sint64 numer, sint64 denom)
{
	return divRndOff64Rate(numer, denom, 1, 2);
}
sint64 divRndOff64Rate(sint64 numer, sint64 denom, sint rndOffRateNumer, sint rndOffRateDenom)
{
	return numer / denom + (((numer % denom) * rndOffRateDenom) / rndOffRateNumer) / denom;
}

uint revEndian(uint value)
{
	return
		value >> 24 & 0x000000ff |
		value >>  8 & 0x0000ff00 |
		value <<  8 & 0x00ff0000 |
		value << 24 & 0xff000000;
}
uint64 revEndian64(uint64 value)
{
	return
		value >> 56 & 0x00000000000000ffui64 |
		value >> 40 & 0x000000000000ff00ui64 |
		value >> 24 & 0x0000000000ff0000ui64 |
		value >>  8 & 0x00000000ff000000ui64 |
		value <<  8 & 0x000000ff00000000ui64 |
		value << 24 & 0x0000ff0000000000ui64 |
		value << 40 & 0x00ff000000000000ui64 |
		value << 56 & 0xff00000000000000ui64;
}

char *zPad64(uint64 value, char *digits, uint minlen)
{
	autoBlock_t *buff = newBlock();
	uint radix = strlen(digits);

	errorCase(radix < 2);

	while (value != 0 || getSize(buff) < minlen)
	{
		addByte(buff, digits[value % radix]);
		value /= radix;
	}
	reverseBytes(buff);
	return unbindBlock2Line(buff);
}
char *zPad(uint value, char *digits, uint minlen)
{
	return zPad64((uint64)value, digits, minlen);
}

// c_
char *c_toLineValue64Digits(uint64 value, char *digits)
{
	static char *stock;
	memFree(stock);
	return stock = toLineValue64Digits(value, digits);
}
char *c_toLineValueDigits(uint value, char *digits)
{
	static char *stock;
	memFree(stock);
	return stock = toLineValueDigits(value, digits);
}
char *c_toLineValue64(uint64 value)
{
	static char *stock;
	memFree(stock);
	return stock = toLineValue64(value);
}
char *c_toLineValue(uint value)
{
	static char *stock;
	memFree(stock);
	return stock = toLineValue(value);
}

// _x
uint64 toValue64Digits_xc(char *line, char *digits)
{
	uint64 out = toValue64Digits(line, digits);
	memFree(line);
	return out;
}
uint toValueDigits_xc(char *line, char *digits)
{
	uint out = toValueDigits(line, digits);
	memFree(line);
	return out;
}
uint64 toValue64_x(char *line)
{
	uint64 out = toValue64(line);
	memFree(line);
	return out;
}
uint toValue_x(char *line)
{
	uint out = toValue(line);
	memFree(line);
	return out;
}
