#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Random.h"

static void Print(autoBlock_t *a, char *title)
{
	uint index;

	cout("%s=", title);

	for (index = getSize(a); index; index--)
		cout("%02x", getByte(a, index - 1));

	cout("\n");
}
static void Norm(autoBlock_t *a)
{
	while (getSize(a) && getByte(a, getSize(a) - 1) == 0)
		unaddByte(a);
}
static void Copy(autoBlock_t *src, autoBlock_t *dest)
{
	setSize(dest, getSize(src));
	memcpy(directGetBuffer(dest), directGetBuffer(src), getSize(src));
}
static int Comp(autoBlock_t *a, autoBlock_t *b)
{
	uint index;

	Norm(a); // 2bs?
	Norm(b); // 2bs?

	if (getSize(a) < getSize(b))
		return -1;

	if (getSize(b) < getSize(a))
		return 1;

	for (index = getSize(a); index; index--)
	{
		if (getByte(a, index - 1) < getByte(b, index - 1))
			return -1;

		if (getByte(b, index - 1) < getByte(a, index - 1))
			return 1;
	}
	return 0;
}

static autoBlock_t *Numer;
static autoBlock_t *Denom;
static autoBlock_t *Ans;
static autoBlock_t *Rem;

// ---- div ----

static autoBlock_t *Dml;

static void DD_Add(uint64 val, uint index)
{
	while (val != 0)
	{
		val += refByte(Ans, index);
		putByte(Ans, index, val & 0xffui64);
		val >>= 8;
		index++;
	}
}
static void DD_Mul(uint64 b)
{
	uint index;

	errorCase((b & 0xff00000000000000ui64) != 0);

	setSize(Dml, 0);

	for (index = 0; index < getSize(Denom); index++)
	{
		uint64 val = getByte(Denom, index) * b;
		uint c = index;

		while (val != 0)
		{
			val += refByte(Dml, c);
			putByte(Dml, c, val & 0xffui64);
			val >>= 8;
			c++;
		}
	}
}
static void DD_Red(uint aPos)
{
	uint index;
	uint val = 1;

	errorCase(Comp(Rem, Dml) < 0);

	for (index = 0; index < getSize(Dml); index++)
	{
		val += getByte(Rem, index + aPos);
		val += 0xff - getByte(Dml, index);
		setByte(Rem, index + aPos, val & 0xff);
		val >>= 8;
	}
	if (!val)
	{
		index += aPos;

		while (!val)
		{
			val += getByte(Rem, index);
			val += 0xff;
			setByte(Rem, index, val & 0xff);
			val >>= 8;
			index++;
		}
	}
	errorCase(val != 1);

	Norm(Rem);
}
static void DoDiv(void)
{
	uint ni;
	uint di;
	uint64 n;
	uint64 d;
	uint64 a;

	Norm(Numer);
	Norm(Denom);

	errorCase(getSize(Numer) <= 8);
	errorCase(getSize(Denom) <= 4);

	Copy(Numer, Rem);

	di = getSize(Denom) - 2;
	d = (uint64)getByte(Denom, di + 0) << 0 |
		(uint64)getByte(Denom, di + 1) << 8;
	d++;

	while (getSize(Denom) + 6 <= getSize(Rem))
	{
		ni = getSize(Rem) - 8;
		n = (uint64)getByte(Rem, ni + 0) <<  0 |
			(uint64)getByte(Rem, ni + 1) <<  8 |
			(uint64)getByte(Rem, ni + 2) << 16 |
			(uint64)getByte(Rem, ni + 3) << 24 |
			(uint64)getByte(Rem, ni + 4) << 32 |
			(uint64)getByte(Rem, ni + 5) << 40 |
			(uint64)getByte(Rem, ni + 6) << 48 |
			(uint64)getByte(Rem, ni + 7) << 56;

		a = n / d;

		DD_Add(a, ni - di);
		DD_Mul(a);
		DD_Red(ni - di);
	}
	while (getSize(Denom) <= getSize(Rem))
	{
		ni = di;
		n = (uint64)refByte(Rem, ni + 0) <<  0 |
			(uint64)refByte(Rem, ni + 1) <<  8 |
			(uint64)refByte(Rem, ni + 2) << 16 |
			(uint64)refByte(Rem, ni + 3) << 24 |
			(uint64)refByte(Rem, ni + 4) << 32 |
			(uint64)refByte(Rem, ni + 5) << 40 |
			(uint64)refByte(Rem, ni + 6) << 48 |
			(uint64)refByte(Rem, ni + 7) << 56;

		a = n / d;

		if (a == 0ui64)
			break;

		DD_Add(a, 0);
		DD_Mul(a);
		DD_Red(0);
	}
	while (Comp(Denom, Rem) <= 0)
	{
		LOGPOS(); // ‚±‚±‚É“ü‚é‚©H‚P‰ñ‚Í“ü‚é‚±‚Æ‚ª‚ ‚è‚»‚¤B‚Q‰ñˆÈã‚Í–³‚¢H

		DD_Add(1, 0);
		Copy(Denom, Dml);
		DD_Red(0);
	}
}

// ---- test ----

static autoBlock_t *Tml;

static void DT_Init(autoBlock_t *dest, uint scale)
{
	uint c;

	setSize(dest, 0);

	for (c = 0; c < scale; c++)
		addByte(dest, mt19937_rnd(256));

	addByte(dest, mt19937_rnd(255) + 1);
}
static void DT_Mul(void)
{
	uint ai;
	uint bi;

	setSize(Tml, 0);

	for (ai = 0; ai < getSize(Ans); ai++)
	for (bi = 0; bi < getSize(Denom); bi++)
	{
		uint val = getByte(Ans, ai) * getByte(Denom, bi);
		uint index = ai + bi;

		while (val)
		{
			val += refByte(Tml, index);
			putByte(Tml, index, val & 0xff);
			val >>= 8;
			index++;
		}
	}
}
static void DT_Add(void)
{
	uint index;

	for (index = 0; index < getSize(Rem); index++)
	{
		uint val = getByte(Rem, index);
		uint c = index;

		while (val)
		{
			val += refByte(Tml, c);
			putByte(Tml, c, val & 0xff);
			val >>= 8;
			c++;
		}
	}
}
static void DoTest(uint scale)
{
	uint ns = mt19937_range(8, scale);
	uint ds = mt19937_range(4, scale);

	DT_Init(Numer, ns);
	DT_Init(Denom, ds);
	setSize(Ans, 0);
	setSize(Rem, 0);

	Print(Numer, "N");
	Print(Denom, "D");

	DoDiv();

	Print(Ans, "A");
	Print(Rem, "R");

	errorCase(Comp(Denom, Rem) <= 0);

	DT_Mul();
	Print(Tml, "t");
	DT_Add();
	Print(Tml, "T");

	errorCase(Comp(Numer, Tml) != 0);

	cout("OK!\n");
}

// ----

int main(int argc, char **argv)
{
	mt19937_init();

	Numer = newBlock();
	Denom = newBlock();
	Ans = newBlock();
	Rem = newBlock();

	Dml = newBlock();

	Tml = newBlock();

	while (!waitKey(0))
	{
		DoTest(10);
		DoTest(30);
		DoTest(100);
		DoTest(300);
		DoTest(1000);
		DoTest(3000);
		DoTest(10000);
	}
}
