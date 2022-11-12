#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Calc.h"

static autoBlock_t *BinStrToBlock(char *str)
{
	autoBlock_t *dest = newBlock();
	char *p;
	uint index;
	uint bit;

	str = strx(str);
	reverseLine(str);
	p = str;

	for (index = 0; *p; index++)
	{
		int chr = 0x00;

		for (bit = 1; bit < 0x100; bit <<= 1)
		{
			switch (*p++)
			{
			case '0':
				break;

			case '1':
				chr |= bit;
				break;

			default:
				error_m(xcout("文字数は8の倍数にしてね。(%u文字)", strlen(str)));
			}
		}
		addByte(dest, chr);
	}
	memFree(str);
	return dest;
}
static void ShowFloat_Main(void *pVal, uint valSz, uint expBits, uint expBase, uint basement, char *strPrintf_x)
{
	char *str = strx("");
	uint index;
	uint bit;
	char *sign;
	char *exp;
	char *fract;

	for (index = 0; index < valSz; index++)
	for (bit = 1; bit < 0x100; bit <<= 1)
	{
		str = addChar(str, ((uchar *)pVal)[index] & bit ? '1' : '0');
	}
	reverseLine(str);

	cout("----\n");
	cout("BINA = %s\n", str);

	sign = strxl(str, 1);
	exp = strxl(str + 1, expBits);
	fract = strx(str + 1 + expBits);

	memFree(str);

	cout("SIGN = %s\n", sign);
	cout("EXP  = %s\n", exp);
	cout("F    = %s\n", fract);

	cout("FORM = (-1 ^ %s) * (2 ^ (b_%s - %u)) * b_1.%s\n", sign, exp, expBase, fract);

	str = calcLine_xx(
		calcPower(
			"-1",
			toValue(sign),
			2
			),
		'*',
		calcLine_xx(
			calcLine_xx(
				calcPower(
					"10",
					toValueDigits(exp, "01"),
					2
					),
				'/',
				calcPower(
					"10",
					expBase,
					2
					),
				2,
				expBase + 30 // + margin
				),
			'*',
			xcout("1.%s", fract),
			2,
			0
			),
		2,
		0
		);

	cout("BVAL = %s\n", str);

	str = changeRadixCalcLine(str, 2, 10, basement);

	if (calcLastMarume)
		str = calcLineToMarume_x(str, basement);

	cout("DVAL = %s\n", str);
	cout("PRNF = %s\n", strPrintf_x);

	memFree(str);
	memFree(strPrintf_x);
}
#if 1
static void ShowFloat(float value)
{
	ShowFloat_Main(&value, 4, 8, 127, 10, xcout("%.10f", value));
}
static void ShowDouble(double value)
{
	ShowFloat_Main(&value, 8, 11, 1023, 20, xcout("%.20f", value));
}
#else
static void ShowFloat(float value)
{
	ShowFloat_Main(&value, 4, 8, 127, 100, xcout("%.100f", value));
}
static void ShowDouble(double value)
{
	ShowFloat_Main(&value, 8, 11, 1023, 400, xcout("%.400f", value));
}
#endif
static void ShowFloat_Block(autoBlock_t *block)
{
	switch (getSize(block))
	{
	case 4:
		ShowFloat(*(float *)b_(block));
		break;

	case 8:
		ShowDouble(*(double *)b_(block));
		break;

	default:
		error();
	}
}
int main(int argc, char **argv)
{
	if (argIs("/NAN"))
	{
		float f = powf(-1.0f, 1.5f);
		double d = pow(-1.0, 1.5);

		ShowFloat(f);
		ShowDouble(d);
		return;
	}
	if (argIs("/INF"))
	{
		float f  = 3.0f;
		double d = 3.0;
		uint c;

		for (c = 100; c; c--)
		{
			f *= f;
			d *= d;
		}
		ShowFloat(f);
		ShowDouble(d);
		ShowFloat(-f);
		ShowDouble(-d);
		return;
	}
	if (argIs("/B"))
	{
		/*
			最初の文字が float の最上位ビット
			最後の文字が float の最下位ビット
		*/
		autoBlock_t *block = BinStrToBlock(nextArg());

		ShowFloat_Block(block);
		releaseAutoBlock(block);
		return;
	}
	if (argIs("/H"))
	{
		/*
			最初の文字が float の最下位バイトの上位 4 ビット
			最後の文字が float の最上位バイトの下位 4 ビット
		*/
		autoBlock_t *block = makeBlockHexLine(nextArg());

		ShowFloat_Block(block);
		releaseAutoBlock(block);
		return;
	}

	{
		char *arg = nextArg();
		float f;
		double d;

		sscanf(arg, "%f", &f);
		sscanf(arg, "%lf", &d);

		ShowFloat(f);
		ShowDouble(d);
	}
}
