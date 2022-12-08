/*
	•¶š
		0 ` 35  -> "0" ` "9" "a" ` "z"
		0 ` 255 -> "[0]" ` "[256]"
		•‰•„†   -> "-"
		¬”“_   -> "."

	¸“x
		‰ÁZ     -> ŠÛ‚ß–³‚µ
		Œ¸Z     -> ŠÛ‚ß–³‚µ
		æZ     -> ŠÛ‚ß–³‚µ
		œZ     -> 1 / (radix ^ basement) ’[”Ø‚èÌ‚Ä, 0 ‚Ö‚ÌŠÛ‚ß
		Šî”•ÏŠ· -> 1 / (radix ^ basement) ’[”Ø‚èÌ‚Ä, 0 ‚Ö‚ÌŠÛ‚ß
		‚×‚«æ   -> ŠÛ‚ß–³‚µ
		‚×‚«ª   -> 1 / (radix ^ basement) ’[”Ø‚èÌ‚Ä, 0 ‚Ö‚ÌŠÛ‚ß
		‘Î”     -> 1 / 1                  ’[”Ø‚èÌ‚Ä, 0 ‚Ö‚ÌŠÛ‚ß

	“Áê‚È‰ğ
		x / 0   -> 0
		x L `1 -> 0
		`1 L x -> 0
		‘Î”‚Ì‰ğ‚ª2^32ˆÈã‚É‚È‚éê‡ -> •s’è
*/

#include "Calc.h"

#define RADIX_MIN 2
#define RADIX_MAX 256

uint calcRadix = 10; // RADIX_MIN ` RADIX_MAX
uint calcBasement;
uint calcBracketedDecimalMin = 36; // 0 ` 36

int calcLastMarume; // ? ÅŒã‚ÌuœZbŠî”•ÏŠ·b‚×‚«ªb‘Î”v‚ÅŠÛ‚ß‚ğs‚Á‚½B

/*
	“Á‚Éª‹’‚Ì–³‚¢ãŒÀ
	ˆ—ŠÔ‚âƒƒ‚ƒŠ“I‚ÈãŒÀ‚Æ‚¢‚¤‚æ‚èAŒ…‚ ‚Ó‚ê‘Îô
*/
#define BASEMENT_MAX IMAX

static uint CheckOp(calcOperand_t *op)
{
	uint index;

	if (calcRadix < RADIX_MIN || RADIX_MAX < calcRadix)
		return 0;

	if (BASEMENT_MAX < calcBasement)
		return 0;

	if (!op)
		return 0;

	for (index = 0; index < getSize(op->Figures); index++)
	{
		if (calcRadix <= getByte(op->Figures, index))
			return 0;
	}
	return 1;
}
static void TrimOp(calcOperand_t *op)
{
	while (getSize(op->Figures) && getByte(op->Figures, 0) == 0 && op->DecIndex)
	{
		desertByte(op->Figures, 0);
		op->DecIndex--;
	}
	while (getSize(op->Figures) && getByte(op->Figures, getSize(op->Figures) - 1) == 0)
	{
		unaddByte(op->Figures);
	}
	if (getSize(op->Figures) == 0) // ? op == "0"
	{
		op->DecIndex = 0;

		if (!calcLastMarume) // ŠÛ‚ß‚ª”­¶‚µ‚½‚Ì‚È‚çop‚Íƒ[ƒ‚Å‚Í‚È‚¢‚Ì‚Å•„†‚ğ•Ï‚¦‚Ä‚Í‚È‚ç‚È‚¢B
			op->Sign = 1;
	}
}
static void ExpandOp(calcOperand_t *op)
{
	insertByte(op->Figures, 0, 0);
	op->DecIndex++;
}
static void ToSameDecIndex(calcOperand_t *op1, calcOperand_t *op2)
{
	while (op1->DecIndex < op2->DecIndex) ExpandOp(op1);
	while (op2->DecIndex < op1->DecIndex) ExpandOp(op2);
}
static void AddInt(calcOperand_t *op, uint index, uint value)
{
	for (; ; )
	{
		value += refByte(op->Figures, index);
		putByte(op->Figures, index, value % calcRadix);

		value /= calcRadix;

		if (!value)
			break;

		index++;
	}
}

calcOperand_t *addCalc(calcOperand_t *op1, calcOperand_t *op2)
{
	calcOperand_t *ans;
	uint index;
	uint opSize;

	errorCase(!CheckOp(op1));
	errorCase(!CheckOp(op2));

	if (op1->Sign == -1)
	{
		op1->Sign = 1;
		ans = subCalc(op2, op1);
		op1->Sign = -1;

		goto endfunc;
	}
	if (op2->Sign == -1)
	{
		op2->Sign = 1;
		ans = subCalc(op1, op2);
		op2->Sign = -1;

		goto endfunc;
	}

	TrimOp(op1);
	TrimOp(op2);
	ToSameDecIndex(op1, op2);

	opSize = m_max(getSize(op1->Figures), getSize(op2->Figures));
	ans = makeCalcOperand("0");

	for (index = 0; index < opSize; index++)
	{
		AddInt(ans, index, refByte(op1->Figures, index) + refByte(op2->Figures, index));
	}
	ans->DecIndex = op1->DecIndex;

endfunc:
	TrimOp(ans);
	return ans;
}
calcOperand_t *subCalc(calcOperand_t *op1, calcOperand_t *op2)
{
	calcOperand_t *ans;
	uint index;
	uint opSize;

	errorCase(!CheckOp(op1));
	errorCase(!CheckOp(op2));

	if (op1->Sign == -1)
	{
		op1->Sign = 1;
		ans = addCalc(op1, op2);
		ans->Sign *= -1;
		op1->Sign = -1;

		goto endfunc;
	}
	if (op2->Sign == -1)
	{
		op2->Sign = 1;
		ans = addCalc(op1, op2);
		op2->Sign = -1;

		goto endfunc;
	}

	TrimOp(op1);
	TrimOp(op2);
	ToSameDecIndex(op1, op2);

	opSize = m_max(getSize(op1->Figures), getSize(op2->Figures));
	opSize = m_max(opSize, 1);
	ans = makeCalcOperand("0");

	for (index = 0; index < opSize; index++)
	{
		AddInt(ans, index, refByte(op1->Figures, index) + calcRadix - refByte(op2->Figures, index) - (index ? 1 : 0));
	}
	if (refByte(ans->Figures, index) == 0) // ? op1 < op2
	{
		releaseCalcOperand(ans);
		ans = subCalc(op2, op1);
		ans->Sign = -1;

		goto endfunc;
	}
	putByte(ans->Figures, index, 0);

	ans->DecIndex = op1->DecIndex;

endfunc:
	TrimOp(ans);
	return ans;
}
calcOperand_t *mulCalc(calcOperand_t *op1, calcOperand_t *op2)
{
	calcOperand_t *ans = makeCalcOperand("0");
	uint index1;
	uint index2;

	errorCase(!CheckOp(op1));
	errorCase(!CheckOp(op2));

	TrimOp(op1);
	TrimOp(op2);

	for (index1 = 0; index1 < getSize(op1->Figures); index1++)
	for (index2 = 0; index2 < getSize(op2->Figures); index2++)
	{
		AddInt(ans, index1 + index2, refByte(op1->Figures, index1) * refByte(op2->Figures, index2));
	}
	ans->DecIndex = op1->DecIndex + op2->DecIndex;
	ans->Sign = op1->Sign * op2->Sign;

	TrimOp(ans);
	return ans;
}
calcOperand_t *divCalc(calcOperand_t *op1, calcOperand_t *op2) // set calcLastMarume
{
	calcOperand_t *ans = makeCalcOperand("0");
	uint shiftCnt;
	uint index;

	errorCase(!CheckOp(op1));
	errorCase(!CheckOp(op2));

	calcLastMarume = 0;

	TrimOp(op1);
	TrimOp(op2);

	if (getSize(op2->Figures) == 0) // ? op1 / 0 == 0
		return ans;

	ans->Sign = op1->Sign * op2->Sign;

	op1 = copyCalcOperand(op1);
	op2 = copyCalcOperand(op2);

	op1->Sign = 1;
	op2->Sign = 1;
	op2->DecIndex += calcBasement;

	TrimOp(op2);
	ToSameDecIndex(op1, op2);

	op1->DecIndex = 0;
	op2->DecIndex = 0;

	shiftCnt = getSize(op2->Figures) < getSize(op1->Figures) ? getSize(op1->Figures) - getSize(op2->Figures) : 0;

	for (index = 0; index < shiftCnt; index++)
		insertByte(op2->Figures, 0, 0);

	for (; ; )
	{
		calcOperand_t *tmpans = subCalc(op1, op2);

		if (tmpans->Sign == 1) // ? 0 <= tmpans
		{
			AddInt(ans, shiftCnt, 1);

			releaseCalcOperand(op1);
			op1 = tmpans;
		}
		else
		{
			releaseCalcOperand(tmpans);

			if (!shiftCnt)
				break;

			op2->DecIndex++;
			shiftCnt--;
		}
	}
	ans->DecIndex = calcBasement;

	calcLastMarume = getSize(op1->Figures);

	releaseCalcOperand(op1);
	releaseCalcOperand(op2);

	TrimOp(ans);
	return ans;
}
sint compCalc(calcOperand_t *op1, calcOperand_t *op2) // ret: (-1, 0, 1) as strcmp()-ly condition
{
	calcOperand_t *ans = subCalc(op1, op2);
	sint ret;

	if (ans->Sign == -1) // ? op1 < op2
	{
		ret = -1;
	}
	else if (getSize(ans->Figures) == 0) // ? op1 == op2
	{
		ret = 0;
	}
	else // op1 > op2
	{
		ret = 1;
	}
	releaseCalcOperand(ans);
	return ret;
}

calcOperand_t *makeCalcOperand(char *line)
{
	calcOperand_t *op = (calcOperand_t *)memAlloc(sizeof(calcOperand_t));
	char *p;
	uint fndPeriod = 0;

	op->Figures = createBlock(16);
	op->DecIndex = 0;
	op->Sign = 1;

	errorCase(!line);

	for (p = line; *p; p++)
	{
		uint d = *p;

		if (d == '-')
		{
			op->Sign = -1;
			continue;
		}
		if (d == '.')
		{
			fndPeriod = 1;
			continue;
		}

		if (m_isdecimal(d))
		{
			d -= '0';
		}
		else if (m_isalpha(d))
		{
			d = m_tolower(d) - 'a' + 10;
		}
		else if (d == '[') // ? "[123]"
		{
			d = 0;

			while (*p && *p != ']')
			{
				if (m_isdecimal(*p))
				{
					d *= 10;
					d += *p - '0';
				}
				p++;
			}
			if (!*p)
				break;
		}
		else
		{
			continue; // Unknown char
		}

		if (fndPeriod)
			op->DecIndex++;

		addByte(op->Figures, d);
	}
	reverseBytes(op->Figures);

	TrimOp(op);
	return op;
}
char *makeLineCalcOperand(calcOperand_t *op)
{
	autoBlock_t *lineBuff = createBlock(16);
	uint index;
	uint opSize;

	errorCase(36 < calcBracketedDecimalMin);

	errorCase(!CheckOp(op));
	TrimOp(op);

	opSize = m_max(op->DecIndex + 1, getSize(op->Figures));

	for (index = 0; index < opSize; index++)
	{
		uint d;

		if (index && index == op->DecIndex)
			addByte(lineBuff, '.');

		d = refByte(op->Figures, index);

		if (calcBracketedDecimalMin <= d) // 36 ` 255
		{
			char *tmpLine;
			autoBlock_t tmpBlock;

			tmpLine = xcout("[%u]", d);
			reverseLine(tmpLine);

			addBytes(lineBuff, gndBlockLineVar(tmpLine, tmpBlock)); memFree(tmpLine);
		}
		else if (10 <= d) // 10 ` 35
		{
			addByte(lineBuff, d + 'a' - 10);
		}
		else // 0 ` 9
		{
			addByte(lineBuff, d + '0');
		}
	}
	if (op->Sign == -1)
	{
		addByte(lineBuff, '-');
	}
	reverseBytes(lineBuff);
	addByte(lineBuff, '\0');

	return unbindBlock(lineBuff);
}
calcOperand_t *copyCalcOperand(calcOperand_t *op)
{
	calcOperand_t *i = (calcOperand_t *)memAlloc(sizeof(calcOperand_t));

	*i = *op;
	i->Figures = copyAutoBlock(i->Figures);
	return i;
}
void releaseCalcOperand(calcOperand_t *op)
{
	releaseAutoBlock(op->Figures);
	memFree(op);
}

/*
	basement
		Š„‚èZˆÈŠO‚Å‚Íg—p‚µ‚Ü‚¹‚ñ‚ªAƒ`ƒFƒbƒN‚Í’Ê‚é‚Ì‚ÅA“Á‚Éw’è‚·‚é’l‚ª–³‚¯‚ê‚Î 0 ‚ğw’è‚µ‚Ä‰º‚³‚¢B
*/
char *calcLine(char *line1, int operator, char *line2, uint radix, uint basement)
{
	calcOperand_t *op1;
	calcOperand_t *op2;
	calcOperand_t *ans;
	char *ansLine;

	errorCase(!line1);
	errorCase(!line2);

	calcRadix = radix;
	calcBasement = basement;

	op1 = makeCalcOperand(line1);
	op2 = makeCalcOperand(line2);

	switch (operator)
	{
	case '+': ans = addCalc(op1, op2); break;
	case '-': ans = subCalc(op1, op2); break;
	case '*': ans = mulCalc(op1, op2); break;
	case '/': ans = divCalc(op1, op2); break;

	default:
		error();
	}
	ansLine = makeLineCalcOperand(ans);

	releaseCalcOperand(op1);
	releaseCalcOperand(op2);
	releaseCalcOperand(ans);

	return ansLine;
}
char *trimCalcLine(char *line, uint radix) // line ‚Ì³‹K‰»
{
	calcOperand_t *op;

	calcRadix = radix;
	calcBasement = 0;

	op = makeCalcOperand(line);
	line = makeLineCalcOperand(op);

	releaseCalcOperand(op);
	return line;
}
sint compCalcLine(char *line1, char *line2, uint radix)
{
	calcOperand_t *op1;
	calcOperand_t *op2;
	sint ret;

	errorCase(!line1);
	errorCase(!line2);

	calcRadix = radix;
	calcBasement = 0;

	op1 = makeCalcOperand(line1);
	op2 = makeCalcOperand(line2);

	ret = compCalc(op1, op2);

	releaseCalcOperand(op1);
	releaseCalcOperand(op2);

	return ret;
}
char *changeRadixCalcLine(char *line, uint radix, uint newRadix, uint basement) // set calcLastMarume
{
	calcOperand_t *newRdxOp;
	calcOperand_t *newOp;
	calcOperand_t *op;
	sint newSign;
	uint index;
	uint shiftCnt;
	int marumeBk;

	errorCase(!line);
	errorCase(radix < RADIX_MIN || RADIX_MAX < radix);
	errorCase(newRadix < RADIX_MIN || RADIX_MAX < newRadix);
	errorCase(BASEMENT_MAX < basement);

	calcRadix = radix;
	calcBasement = 0;

	newRdxOp = makeCalcOperand("0");
	AddInt(newRdxOp, 0, newRadix);
	newOp = makeCalcOperand("0");

	op = makeCalcOperand(line);

	newSign = op->Sign;
	op->Sign = 1;

	for (index = 0; index < basement; index++)
	{
		calcOperand_t *tmpOp;
		op = mulCalc(tmpOp = op, newRdxOp); releaseCalcOperand(tmpOp);
	}

	{
		calcOperand_t *one = makeCalcOperand("1");
		calcOperand_t *tmpOp;

		op = divCalc(tmpOp = op, one);
		marumeBk = calcLastMarume;

		releaseCalcOperand(one);
		releaseCalcOperand(tmpOp);
	}

	shiftCnt = 0;

	while (getSize(op->Figures)) // ? op != 0
	{
		calcOperand_t *wop1;
		calcOperand_t *wop2;
		calcOperand_t *wop3;
		uint value;

		calcRadix = radix;
		wop1 = divCalc(op, newRdxOp);
		wop2 = mulCalc(wop1, newRdxOp);
		wop3 = subCalc(op, wop2);

		errorCase(8 < getSize(wop3->Figures));
		errorCase(wop3->DecIndex != 0);
		errorCase(wop3->Sign != 1);

		value = 0;

		for (index = 7; ; index--)
		{
			value *= radix;
			value += refByte(wop3->Figures, index);

			if (!index)
				break;
		}
		calcRadix = newRadix;
		AddInt(newOp, shiftCnt, value);

		releaseCalcOperand(op);
		releaseCalcOperand(wop2);
		releaseCalcOperand(wop3);
		op = wop1;

		shiftCnt++;
	}
	newOp->DecIndex = basement;
	newOp->Sign = newSign;

	line = makeLineCalcOperand(newOp);

//	calcRadix = newRadix;
	calcLastMarume = marumeBk;

	releaseCalcOperand(newRdxOp);
	releaseCalcOperand(newOp);
	releaseCalcOperand(op);

	return line;
}

char *calcPower(char *line, uint exponent, uint radix)
{
	char *ansLine;
	char *tmpLine;

	errorCase(!line);
	errorCase(radix < RADIX_MIN || RADIX_MAX < radix);

	if (exponent == 0) return strx("1");
	if (exponent == 1) return trimCalcLine(line, radix);

	tmpLine = calcPower(line, exponent / 2, radix);
	ansLine = calcLine(tmpLine, '*', tmpLine, radix, 0);
	memFree(tmpLine);

	if (exponent & 1)
	{
		ansLine = calcLine(tmpLine = ansLine, '*', line, radix, 0);
		memFree(tmpLine);
	}
	return ansLine;
}
char *calcRootPower(char *line, uint exponent, uint radix, uint basement) // set calcLastMarume
{
	char *ansLine = strx("0");
	char *expLine;
	char *anxLine;
	char *tmpLine;

	errorCase(!line);
	errorCase(exponent < 2);
	errorCase(radix < RADIX_MIN || RADIX_MAX < radix);
	errorCase(BASEMENT_MAX < basement);

	line = trimCalcLine(line, radix);
	if (line[0] == '-') eraseChar(line); // line = abs(line);

	// make expLine
	{
		expLine = strx("1");

		for (; ; )
		{
			tmpLine = addChar(strx(expLine), '0');

			if (compCalcLine(line, tmpLine, radix) == -1) // ? line < tmpLine
				break;

			memFree(expLine);
			expLine = tmpLine;
		}
		memFree(tmpLine);
	}

	calcLastMarume = 1;

	while (strcmp(expLine, "0")) // ? expLine != 0
	{
		sint ret;

		anxLine = calcLine(ansLine, '+', expLine, radix, 0);
		tmpLine = calcPower(anxLine, exponent, radix);

		ret = compCalcLine(line, tmpLine, radix);

		if (ret == -1) // ? line < tmpLine
		{
			memFree(anxLine);
			memFree(tmpLine);

			expLine = calcLine(tmpLine = expLine, '/', "10", radix, basement); // Ÿ‚ÌŒ…Abasement ‚ğ’´‚¦‚½‚ç 0
			memFree(tmpLine);
		}
		else
		{
			memFree(ansLine);
			memFree(tmpLine);

			ansLine = anxLine;

			if (!ret)
			{
				calcLastMarume = 0;
				break;
			}
		}
	}
	memFree(line);
	memFree(expLine);

	return ansLine;
}
static uint GetLogarithm(char *line1, char *line2, char **p_expLine, uint radix)
{
	char *pLine;
	char *qLine;
	char *rLine;
	uint exponent;

	if (compCalcLine(line1, line2, radix) == -1) // ? line1 < line2
	{
		*p_expLine = NULL;
		return 0;
	}
	pLine = calcLine(line2, '*', line2, radix, 0);
	exponent = GetLogarithm(line1, pLine, &qLine, radix);

	if (!exponent)
	{
		*p_expLine = strx(line2);
		return 1;
	}
	rLine = calcLine(qLine, '*', line2, radix, 0);
	exponent *= 2;

	if (compCalcLine(line1, rLine, radix) != -1) // ? rLine <= line1
	{
		memFree(qLine);
		qLine = rLine;
		exponent++;
	}
	else
	{
		memFree(rLine);
	}
	*p_expLine = qLine;
	return exponent;
}
uint calcLogarithm(char *line1, char *line2, uint radix) // line2: ’ê, set calcLastMarume
{
	char *expLine;
	uint exponent;

	errorCase(!line1);
	errorCase(!line2);
	errorCase(radix < RADIX_MIN || RADIX_MAX < radix);

	line1 = trimCalcLine(line1, radix);
	line2 = trimCalcLine(line2, radix);
//	if (line1[0] == '-') eraseChar(line1); // line1 = abs(line1);
//	if (line2[0] == '-') eraseChar(line2); // line2 = abs(line2);

	if (
		compCalcLine(line1, "1", radix) <= 0 ||
		compCalcLine(line2, "1", radix) <= 0
//		!strcmp(line1, "0") || !strcmp(line1, "1") ||
//		!strcmp(line2, "0") || !strcmp(line2, "1")
		)
	{
		exponent = 0;
		calcLastMarume = 0;
	}
	else
	{
		exponent = GetLogarithm(line1, line2, &expLine, radix);

		if (expLine)
		{
			calcLastMarume = strcmp(line1, expLine);
			memFree(expLine);
		}
		else
		{
			calcLastMarume = 1;
		}
	}
	memFree(line1);
	memFree(line2);

	return exponent;
}

/*
	•\¦—p‚ÉŠÛ‚ß‚ğs‚Á‚½‚æ‚ÉŒ©‚¦‚é•¶š—ñ‚É‚·‚éB
*/
char *calcLineToMarume(char *line, uint basement)
{
	char *p;
	uint bLen;

	line = strx(line);
	p = strchr(line, '.');

	if (!p)
	{
		line = addChar(line, '.');
		p = strchr(line, '.');
	}
	for (bLen = strlen(p + 1); bLen < basement; bLen++)
		line = addChar(line, '0');

	line = addChar(line, '*');
	return line;
}

// _x
char *calcLine_cx(char *line1, int operator, char *line2, uint radix, uint basement)
{
	char *out = calcLine(line1, operator, line2, radix, basement);
	memFree(line2);
	return out;
}
char *calcLine_xc(char *line1, int operator, char *line2, uint radix, uint basement)
{
	char *out = calcLine(line1, operator, line2, radix, basement);
	memFree(line1);
	return out;
}
char *calcLine_xx(char *line1, int operator, char *line2, uint radix, uint basement)
{
	char *out = calcLine(line1, operator, line2, radix, basement);
	memFree(line1);
	memFree(line2);
	return out;
}
char *changeRadixCalcLine_x(char *line, uint radix, uint newRadix, uint basement)
{
	char *out = changeRadixCalcLine(line, radix, newRadix, basement);
	memFree(line);
	return out;
}
char *calcPower_x(char *line, uint exponent, uint radix)
{
	char *out = calcPower(line, exponent, radix);
	memFree(line);
	return out;
}
char *calcRootPower_x(char *line, uint exponent, uint radix, uint basement)
{
	char *out = calcRootPower(line, exponent, radix, basement);
	memFree(line);
	return out;
}
char *calcLineToMarume_x(char *line, uint basement)
{
	char *out = calcLineToMarume(line, basement);
	memFree(line);
	return out;
}
