/*
	12 * 34 + 56 / 78
		Keisan.exe 12 * 34 = L ; 56 / 79 = R ; @L + @R
		Keisan.exe 56 / 79 = R ; 12 * 34 + @R

	12 - 34 / (56 + 78)
		Keisan.exe 56 + 78 !/ 34 !- 12

	‚Q‚Ì•½•ûª‚ğ¬”‘æ100ˆÊ‚Ü‚Å‹‚ß‚éB
		Keisan.exe /b 100 2 r 2

	‚P‚O‚O‚ÌŠKæ
		Keisan.exe 100 u 100

	ˆ—       ‹L†   ¶’l       ‰E’l       •]‰¿’l     ¸“x
	-----------------------------------------------------------
	‰ÁZ       +      ‘½”{’·     ‘½”{’·     ‘½”{’·     ŠÛ‚ß–³‚µ
	Œ¸Z       -      ‘½”{’·     ‘½”{’·     ‘½”{’·     ŠÛ‚ß–³‚µ
	æZ       *      ‘½”{’·     ‘½”{’·     ‘½”{’·     ŠÛ‚ß–³‚µ
	œZ       /      ‘½”{’·     ‘½”{’·     ‘½”{’·     ­”‘æ basement ˆÊ‚Ü‚Å, ­”‘æ basement + 1 ˆÊˆÈ‰ºØ‚èÌ‚Ä
	è—]       M      ‘½”{’·     ‘½”{’·     ‘½”{’·     ŠÛ‚ß–³‚µ
	‚×‚«æ     P      ‘½”{’·     10iuint   ‘½”{’·     ŠÛ‚ß–³‚µ
	‚×‚«ª     R      ‘½”{’·     10iuint   ‘½”{’·     ­”‘æ basement ˆÊ‚Ü‚Å, ­”‘æ basement + 1 ˆÊˆÈ‰ºØ‚èÌ‚Ä
	‘Î”       L      ‘½”{’·     ‘½”{’·     10iuint   ®”,                   ­”‘æ 1 ˆÊˆÈ‰ºØ‚èÌ‚Ä
	i”•ÏŠ·   X      ‘½”{’·     10iuint   ‘½”{’·     ­”‘æ basement ˆÊ‚Ü‚Å, ­”‘æ basement + 1 ˆÊˆÈ‰ºØ‚èÌ‚Ä
	‡—ñ       U      10iuint   10iuint   ‘½”{’·     ŠÛ‚ß–³‚µ
	‘g‡‚¹     C      10iuint   10iuint   ‘½”{’·     ŠÛ‚ß–³‚µ
	Ø‚èÌ‚Ä   I      ‘½”{’·     10iuint   ‘½”{’·     ¬”‘æ '‰E’l' ˆÊ‚Ü‚Å, ­”‘æ '‰E’l' + 1 ˆÊˆÈ‰ºØ‚èÌ‚Ä

	'10iuint' ‚Í radix, basement ‚ÉŠÖ‚í‚ç‚¸ 10 i” 0 ` 4294967295 ‚Ì®”

	“Áê‚È‰ğ
		x / 0   -> 0
		`1 L x -> 0
		x L `1 -> 0
		‘Î”‚Ì‰ğ‚ª 2^32 ˆÈã‚Ìê‡ -> •s’è
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\BigInt.h"
#include "C:\Factory\Common\Options\Calc.h"

static char *LeftPad(char *line)
{
	if (strlen(line) & 1)
	{
		static char *padLine;

		memFree(padLine);
		padLine = xcout("0%s", line);
		line = padLine;
	}
	return line;
}
static autoList_t *MakeHexOp(char *line)
{
	autoBlock_t *bop = makeBlockHexLine(LeftPad(line));
	autoList_t *op = newList();
	uint index;
	uint value;

	reverseBytes(bop);

	for (index = 0; index < (getSize(bop) + 3) / 4; index++)
	{
		value = refByte(bop, index * 4) +
			refByte(bop, index * 4 + 1) * 0x100 +
			refByte(bop, index * 4 + 2) * 0x10000 +
			refByte(bop, index * 4 + 3) * 0x1000000;

		addElement(op, value);
	}
	releaseAutoBlock(bop);
	return op;
}
static void DispHexOp(autoList_t *op, uint bound, char *title)
{
	uint index;
	uint value;

	op = copyAutoList(op);

	if (getCount(op) < bound)
	{
		setCount(op, bound);
	}
	reverseElements(op);
	cout("%s: ", title);

	foreach (op, value, index)
	{
		if (index)
		{
			cout(":");
		}
		cout("%08x", value);
	}
	cout("\n");
	releaseAutoList(op);
}
static void HexKeisan(void)
{
	uint bound = 8;
	autoList_t *op1;
	autoList_t *op2;
	autoList_t *op3;
	autoList_t *ans;
	int op;

// readArgs:
	if (argIs("/B")) // Bound
	{
		bound = toValue(nextArg());
//		goto readArgs;
	}

	op1 = MakeHexOp(nextArg());

	while (hasArgs(2))
	{
		op = nextArg()[0];
		op2 = MakeHexOp(nextArg());

		cout("OPERATOR: %c\n", op);
		DispHexOp(op1, bound, "OP-001");
		DispHexOp(op2, bound, "OP-002");

		switch (m_tolower(op))
		{
		case '+': ans = addBigInt(op1, op2); break;
		case '-':
			ans = subBigInt(op1, op2);
			errorCase(!ans);
			break;

		case '*': ans = mulBigInt(op1, op2); break;
		case '/': ans = divBigInt(op1, op2, NULL); break;
		case 'm': ans = modBigInt(op1, op2); break;
		case 'p': ans = powerBigInt(op1, op2); break;
		case 'o':
			op3 = MakeHexOp(nextArg());
			DispHexOp(op3, bound, "OP-003");
			ans = modPowerBigInt(op1, op2, op3);
			releaseAutoList(op3);
			break;

		default:
			error();
		}
		DispHexOp(ans, bound, "ANSWER");

		releaseAutoList(op1);
		releaseAutoList(op2);

		op1 = ans;
	}
	releaseAutoList(op1);
}

static autoList_t *MemoryKeys;
static autoList_t *MemoryValues;

static void InitMemory(void)
{
	MemoryKeys = newList();
	MemoryValues = newList();
}
static void FnlzMemory(void)
{
	releaseDim(MemoryKeys, 1);
	releaseDim(MemoryValues, 1);
}
static void SetMemory(char *key, char *value)
{
	uint index = findLineCase(MemoryKeys, key, 1);

	if (index < getCount(MemoryKeys))
	{
		memFree(getLine(MemoryValues, index));
		setElement(MemoryValues, index, (uint)strx(value));
	}
	else
	{
		addElement(MemoryKeys, (uint)strx(key));
		addElement(MemoryValues, (uint)strx(value));
	}
}
static char *GetMemory(char *key)
{
	return getLine(MemoryValues, findLineCase(MemoryKeys, key, 1));
}

static char *InsSepOp(char *line, uint ranks)
{
	if (ranks) // x , n -> x ‚É n Œ…‚İ‚Å ',' ‚ğ“ü‚ê‚éB
	{
		uint decpidx = (uint)strchrEnd(line, '.') - (uint)line;
		uint index;

		for (index = decpidx + ranks + 1; index < strlen(line); index += ranks + 1)
			line = insertChar(line, index, ',');

		for (index = decpidx; ranks < index; )
		{
			index -= ranks;
			line = insertChar(line, index, ',');
		}
		if (startsWith(line, "-,"))
			eraseChar(line + 1);
	}
	else // x , 0 -> ’·‚³‚ğ•Ô‚·B
	{
		char *swrk = xcout("%u", strlen(line));
		memFree(line);
		line = swrk;
	}
	return line;
}
static char *Permutation(uint v1, uint v2, uint radix)
{
	char *ans = strx("1");
	uint count;

	errorCase(!m_isRange(v1, 1, UINTMAX));
	errorCase(!m_isRange(v2, 1, v1));

	for (count = v1 - v2 + 1; ; count++)
	{
		ans = calcLine_xx(ans, '*', changeRadixCalcLine_x(xcout("%u", count), 10, radix, 0), radix, 0);

		if (v1 <= count)
			break;
	}
	return ans;
}
static char *Combination(uint v1, uint v2, uint radix)
{
	errorCase(!m_isRange(v1, 1, UINTMAX));
	errorCase(!m_isRange(v2, 1, v1));

	return calcLine_xx(Permutation(v1, v2, radix), '/', Permutation(v2, v2, radix), radix, 0);
}
static void Main2(void)
{
	uint radix = 10;
	uint basement = 10;
	char *op1;
	char *pop;
	char *op2;
	char *ans;
	int noShowMarume = 0;
	int onceNoShowMarume = 0;

	InitMemory();

	if (argIs("/H")) // Hex
	{
		HexKeisan();
		goto endFunc;
	}

readArgs:
	if (argIs("/R") || argIs("/X")) // Radix
	{
		radix = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/B")) // Basement
	{
		basement = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/D"))
	{
		calcBracketedDecimalMin = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/-M"))
	{
		noShowMarume = 1;
		goto readArgs;
	}

	op1 = strx(nextArg());

	while (hasArgs(2))
	{
		pop= nextArg();
		op2 = strx(nextArg());
		onceNoShowMarume = 0;

		if (op2[0] == '@')
		{
			ans = strx(GetMemory(op2 + 1));
			memFree(op2);
			op2 = ans;
		}
		calcLastMarume = 0;
	reopration:
		switch (m_tolower(*pop))
		{
		case '!':
			{
				char *swapop = op1;
				op1 = op2;
				op2 = swapop;
			}
			pop++;
			goto reopration;

		case '+': ans = calcLine(op1, '+', op2, radix, 0); break;
		case '-': ans = calcLine(op1, '-', op2, radix, 0); break;
		case '*': ans = calcLine(op1, '*', op2, radix, 0); break;
		case '/': ans = calcLine(op1, '/', op2, radix, basement); break;
		case 'm':
			{
				char *opd = calcLine(op1, '/', op2, radix, 0);
				char *opm = calcLine(opd, '*', op2, radix, 0);
				ans = calcLine(op1, '-', opm, radix, 0); // •„†‚Í op1 ‚Éˆê’v‚·‚éB
				memFree(opd);
				memFree(opm);
				calcLastMarume = 0;
			}
			break;

		case 'p': ans = calcPower(op1, toValue(op2), radix); break;
		case 'r': ans = calcRootPower(op1, toValue(op2), radix, basement); break;
		case 'l': ans = xcout("%u", calcLogarithm(op1, op2, radix)); break;

		case 'b': ans = strx(op1); basement = toValue(op2); break;
		case 'x':
			{
				uint newRadix = toValue(op2);
				ans = changeRadixCalcLine(op1, radix, newRadix, basement);
				radix = newRadix;
			}
			break;

		case 'u': ans = Permutation(toValue(op1), toValue(op2), radix); break;
		case 'c': ans = Combination(toValue(op1), toValue(op2), radix); break;
		case 'i': ans = calcLine(op1, '/', "1", radix, toValue(op2)); onceNoShowMarume = 1; break;
		case '=': ans = strx(op1); SetMemory(op2, op1); break;
		case ';': ans = strx(op2); break;
		case ',': ans = InsSepOp(strx(op1), toValue(op2)); break;

		default:
			error();
		}
		if (calcLastMarume && !noShowMarume && !onceNoShowMarume)
		{
			char *tmp;

			if (m_tolower(*pop) == 'l')
				tmp = calcLineToMarume(ans, 0);
			else
				tmp = calcLineToMarume(ans, basement);

			cout("%s\n", tmp);
			memFree(tmp);
		}
		else
		{
			cout("%s\n", ans);
		}
		memFree(op1);
		memFree(op2);

		op1 = ans;
	}
	memFree(op1);

endFunc:
	FnlzMemory();
}
int main(int argc, char **argv)
{
	Main2();
	termination(0);
}
