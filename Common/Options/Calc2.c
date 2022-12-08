#include "Calc2.h"

uint calcRadix = 10;
sint calcBasement = 30;
int calcScient = 0;
uint calcEffect = 0; // 0 == –³Œø
int calcRndOff = 1;

static void CheckVars(void)
{
	errorCase(!m_isRange(calcRadix, 2, 36));
	errorCase(!m_isRange(calcBasement, -IMAX, IMAX));
	// calcScient
	errorCase(!m_isRange(calcEffect, 0, IMAX));
	// calcRndOff
}

static calcOperand_t *CreateOperand(void)
{
	calcOperand_t *co = (calcOperand_t *)memAlloc(sizeof(calcOperand_t));

	co->F = newBlock();
	co->E = 0;
	co->Sign = 1;

	return co;
}
static void ReleaseOperand(calcOperand_t *co)
{
	releaseAutoBlock(co->F);
	memFree(co);
}

static void Trim(calcOperand_t *co)
{
	while (getSize(co->F) && !getByte(co->F, getSize(co->F) - 1))
	{
		unaddByte(co->F);
	}

#if 1
	{
		uint desCnt = 0;

		while (desCnt < getSize(co->F) && !getByte(co->F, desCnt))
		{
			desCnt++;
			co->E--;
		}
		removeBytes(co->F, 0, desCnt);
	}
#else // same, ’x‚¢..
	while (getSize(co->F) && !getByte(co->F, 0))
	{
		desertByte(co->F, 0);
		co->E--;
	}
#endif

	if (!getSize(co->F))
	{
		co->E = 0;
		co->Sign = 1; // ŠÛ‚ß•\‹L‚É‘Î‰ž‚µ‚È‚¢‚Ì‚ÅACalc.c ‚Ì‚æ‚¤‚É‚Í‚µ‚È‚¢B
	}
}
static void Expand(calcOperand_t *co, uint count)
{
#if 1
	insertByteRepeat(co->F, 0, 0, count);
	co->E += count;
#else // same, ’x‚¢..
	while (count)
	{
		insertByte(co->F, 0, 0);
		co->E++;
		count--;
	}
#endif
}
static void AddInt(calcOperand_t *co, uint index, uint value)
{
	for (; ; )
	{
		value += refByte(co->F, index);
		putByte(co->F, index, value % calcRadix);
		value /= calcRadix;

		if (!value)
			break;

		index++;
	}
}
static calcOperand_t *GetClone(calcOperand_t *co)
{
	calcOperand_t *nCo = (calcOperand_t *)memAlloc(sizeof(calcOperand_t));

	nCo->F = copyAutoBlock(co->F);
	nCo->E = co->E;
	nCo->Sign = co->Sign;

	return nCo;
}

calcOperand_t *calcFromSmplString(char *str)
{
	calcOperand_t *co;
	char *p;
	int fndPrd;

	errorCase(!str);
	CheckVars();

	co = CreateOperand();
	fndPrd = 0;

	for (p = str; *p; p++)
	{
		int chr = *p;

		if (chr == '-')
		{
			co->Sign = -1;
			continue;
		}
		if (chr == '.')
		{
			fndPrd = 1;
			continue;
		}
		if (m_isdecimal(chr) || m_isalpha(chr))
		{
			uint value = m_c2i(chr);

			if (value < calcRadix)
			{
				addByte(co->F, value);

				if (fndPrd)
					co->E++;
			}
		}
	}
	reverseBytes(co->F);
	Trim(co);
	return co;
}
calcOperand_t *calcFromString(char *str)
{
	char *p;
	calcOperand_t *co;

	errorCase(!str);
	CheckVars();

	str = strx(str);

	if (calcRadix < 15) // ? "Ee" are not digit
	{
		(p = strchr(str, 'E')) ||
		(p = strchr(str, 'e'));
	}
	else
	{
		(p = strstr(str, "E+")) ||
		(p = strstr(str, "e+")) ||
		(p = strstr(str, "E-")) ||
		(p = strstr(str, "e-"));
	}

	if (p)
	{
		*p = '\0';
		p++;

		co = calcFromSmplString(str);
		co->E -= atoi(p);
	}
	else
	{
		co = calcFromSmplString(str);
	}
	memFree(str);
	Trim(co);
	return co;
}
calcOperand_t *calcFromInt(sint value)
{
	calcOperand_t *co = CreateOperand();

	errorCase(!m_isRange(value, -IMAX, IMAX));

	if (value < 0)
	{
		AddInt(co, 0, (uint)-value);
		co->Sign = -1;
	}
	else
	{
		AddInt(co, 0, (uint)value);
	}
	return co;
}
void calcRelease(calcOperand_t *co)
{
	errorCase(!co);
	CheckVars();

	ReleaseOperand(co);
}

char *calcGetSmplString_EM(calcOperand_t *co, uint effectMin)
{
	autoBlock_t *buff = newBlock();
	uint index;
	uint iEnd;

	errorCase(!co);
	CheckVars();

	Trim(co);

	if (effectMin)
	{
		while ((sint)getSize(co->F) < co->E + 1)
		{
			addByte(co->F, 0);
		}
#if 1
		if (getSize(co->F) < effectMin)
		{
			Expand(co, effectMin - getSize(co->F));
		}
#else // same, ’x‚¢..
		while (getSize(co->F) < effectMin)
		{
			Expand(co, 1);
		}
#endif
	}
	for (index = (uint)m_max(0, -co->E); index; index--)
	{
		addByte(buff, '0');
	}
	iEnd = (uint)m_max(co->E + 1, (sint)getSize(co->F));

	for (index = 0; index < iEnd; index++)
	{
		if (index && index == co->E)
		{
			addByte(buff, '.');
		}
		addByte(buff, m_i2c(refByte(co->F, index)));
	}
	if (co->Sign == -1)
	{
		addByte(buff, '-');
	}
	reverseBytes(buff);
	return unbindBlock2Line(buff);
}
char *calcGetSmplString(calcOperand_t *co)
{
	return calcGetSmplString_EM(co, 0);
}
char *calcGetString(calcOperand_t *co)
{
	uint index;
	char *ret;

	errorCase(!co);
	CheckVars();

	co = GetClone(co);
	Trim(co);

	if (calcEffect && calcEffect < getSize(co->F))
	{
		uint iEnd = getSize(co->F) - calcEffect;

		for (index = 0; index < iEnd; index++)
		{
			if (calcRndOff && index == iEnd - 1 && calcRadix / 2 <= getByte(co->F, index))
			{
				AddInt(co, iEnd, 1); // ŽlŽÌŒÜ“ü
			}
			setByte(co->F, index, 0);
		}
		Trim(co);
	}
	if (calcScient)
	{
		sint me = m_max(0, (sint)getSize(co->F) - 1);
		sint ee;

		ee = me - co->E;
		co->E = me;

		ret = calcGetSmplString_EM(co, calcEffect);
		ret = addChar(ret, 'E');
		ret = addChar(ret, ee < 0 ? '-' : '+');
		ret = addLine_x(ret, xcout("%02d", abs(ee)));
	}
	else
	{
		ret = calcGetSmplString(co);
	}
	ReleaseOperand(co);
	return ret;
}

static SyncE(calcOperand_t *co1, calcOperand_t *co2)
{
#if 1
	if (co1->E < co2->E)
	{
		Expand(co1, co2->E - co1->E);
	}
	if (co2->E < co1->E)
	{
		Expand(co2, co1->E - co2->E);
	}
#else // same, ’x‚¢..
	while (co1->E < co2->E)
	{
		Expand(co1, 1);
	}
	while (co2->E < co1->E)
	{
		Expand(co2, 1);
	}
#endif
}

calcOperand_t *calcAdd(calcOperand_t *co1, calcOperand_t *co2)
{
	calcOperand_t *ans;
	uint index;
	uint iEnd;

	errorCase(!co1);
	errorCase(!co2);
	CheckVars();

	if (co1 == co2)
	{
		co2 = GetClone(co2);
		ans = calcAdd(co1, co2);
		ReleaseOperand(co2);
		return ans;
	}

	if (co1->Sign == -1)
	{
		co1->Sign = 1;
		ans = calcSub(co2, co1);
		co1->Sign = -1;
		return ans;
	}
	if (co2->Sign == -1)
	{
		co2->Sign = 1;
		ans = calcSub(co1, co2);
		co2->Sign = -1;
		return ans;
	}
	Trim(co1);
	Trim(co2);
	SyncE(co1, co2);

	ans = CreateOperand();
	iEnd = m_max(getSize(co1->F), getSize(co2->F));

	for (index = 0; index < iEnd; index++)
	{
		AddInt(ans, index, refByte(co1->F, index) + refByte(co2->F, index));
	}
	ans->E = co1->E;
	Trim(ans);
	return ans;
}
calcOperand_t *calcSub(calcOperand_t *co1, calcOperand_t *co2)
{
	calcOperand_t *ans;
	uint index;
	uint iEnd;

	errorCase(!co1);
	errorCase(!co2);
	CheckVars();

	if (co1 == co2)
		return CreateOperand(); // x - x == 0

	if (co1->Sign == -1)
	{
		co1->Sign = 1;
		ans = calcAdd(co1, co2);
		ans->Sign *= -1;
		co1->Sign = -1;
		return ans;
	}
	if (co2->Sign == -1)
	{
		co2->Sign = 1;
		ans = calcAdd(co1, co2);
		co2->Sign = -1;
		return ans;
	}
	Trim(co1);
	Trim(co2);
	SyncE(co1, co2);

	ans = CreateOperand();
	iEnd = m_max(getSize(co1->F) , getSize(co2->F));
	m_maxim(iEnd, 1);

	for (index = 0; index < iEnd; index++)
	{
		AddInt(ans, index, refByte(co1->F, index) + calcRadix - refByte(co2->F, index) - (index ? 1 : 0));
	}
	if (!refByte(ans->F, iEnd))
	{
		ReleaseOperand(ans);
		ans = calcSub(co2, co1);
		ans->Sign = -1;
		return ans;
	}
	setByte(ans->F, iEnd, 0);
	ans->E = co1->E;
	Trim(ans);
	return ans;
}
calcOperand_t *calcMul(calcOperand_t *co1, calcOperand_t *co2)
{
	calcOperand_t *ans;
	uint index1;
	uint index2;

	errorCase(!co1);
	errorCase(!co2);
	CheckVars();
	// co1 == co2 -> –â‘è‚È‚¢

	Trim(co1);
	Trim(co2);
	ans = CreateOperand();

	for (index1 = 0; index1 < getSize(co1->F); index1++)
	for (index2 = 0; index2 < getSize(co2->F); index2++)
	{
		AddInt(ans, index1 + index2, getByte(co1->F, index1) * getByte(co2->F, index2));
	}
	ans->E = co1->E + co2->E;
	ans->Sign = co1->Sign * co2->Sign;
	Trim(ans);
	return ans;
}
calcOperand_t *calcDiv(calcOperand_t *co1, calcOperand_t *co2)
{
	calcOperand_t *ans;
	calcOperand_t *tmp;
	sint shiftCnt;

	errorCase(!co1);
	errorCase(!co2);
	CheckVars();
	// co1 == co2 -> •¡»‚·‚é‚©‚ç‘åä•v

	Trim(co1);
	Trim(co2);
	ans = CreateOperand();

	if (!getSize(co2->F)) // ? ƒ[ƒ‚ÅœŽZ
		return ans;

	ans->Sign = co1->Sign * co2->Sign;

	co1 = GetClone(co1);
	co2 = GetClone(co2);
	co1->Sign = 1;
	co2->Sign = 1;
	co2->E += calcBasement;
	Trim(co2);
	SyncE(co1, co2);

	shiftCnt = m_max(0, (sint)getSize(co1->F) - (sint)getSize(co2->F));

	co2->E -= shiftCnt;

	while (0 <= shiftCnt)
	{
		tmp = calcSub(co1, co2);

		if (tmp->Sign == 1)
		{
			AddInt(ans, shiftCnt, 1);
			ReleaseOperand(co1);
			co1 = tmp;
		}
		else
		{
			ReleaseOperand(tmp);
			co2->E++;
			shiftCnt--;
		}
	}
	ans->E = calcBasement;
	Trim(ans);
	ReleaseOperand(co1);
	ReleaseOperand(co2);
	return ans;
}

char *calc(char *str1, int op, char *str2)
{
	calcOperand_t *co1;
	calcOperand_t *co2;
	calcOperand_t *ans;
	char *ret;

	errorCase(!str1);
	errorCase(!str2);

	co1 = calcFromString(str1);
	co2 = calcFromString(str2);

	switch (op)
	{
	case '+':
		ans = calcAdd(co1, co2);
		break;

	case '-':
		ans = calcSub(co1, co2);
		break;

	case '*':
		ans = calcMul(co1, co2);
		break;

	case '/':
		ans = calcDiv(co1, co2);
		break;

	default:
		error();
	}
	ret = calcGetString(ans);

	ReleaseOperand(co1);
	ReleaseOperand(co2);
	ReleaseOperand(ans);

	return ret;
}

int calcComp(calcOperand_t *co1, calcOperand_t *co2)
{
	int s;
	uint size1;
	uint size2;
	sint scale1;
	sint scale2;
	uint index1;
	uint index2;
	int ret;

	errorCase(!co1);
	errorCase(!co2);

	Trim(co1);
	Trim(co2);

	if (co1->Sign == 1 && co2->Sign == -1)
		return 1;

	if (co1->Sign == -1 && co2->Sign == 1)
		return -1;

	s = co1->Sign;

	size1 = getSize(co1->F);
	size2 = getSize(co2->F);

	if (!size1 && !size2)
		return 0;

	if (!size1)
		return -s;

	if (!size2)
		return s;

	scale1 = (sint)size1 - co1->E;
	scale2 = (sint)size2 - co2->E;

	if (scale1 < scale2)
		return -s;

	if (scale2 < scale1)
		return s;

	index1 = getSize(co1->F);
	index2 = getSize(co2->F);

	while (index1 && index2)
	{
		index1--;
		index2--;

		ret = (int)getByte(co1->F, index1) - (int)getByte(co2->F, index2);

		if (ret != 0)
			return ret * s;
	}
	if (index1)
		return s;

	if (index2)
		return -s;

	return 0;
}
calcOperand_t *calcPower(calcOperand_t *co, uint exponent)
{
	errorCase(!co);
	// exponent
	CheckVars();

	if (!exponent)
	{
		co = CreateOperand();
		AddInt(co, 0, 1);
		return co;
	}
	if (exponent == 1)
		return GetClone(co);

	{
		calcOperand_t *ans = calcMul(co, co);
		calcOperand_t *tmp;

		if (4 <= exponent)
		{
			tmp = calcPower(ans, exponent / 2);
			ReleaseOperand(ans);
			ans = tmp;
		}
		if (exponent & 1)
		{
			tmp = calcMul(ans, co);
			ReleaseOperand(ans);
			ans = tmp;
		}
		return ans;
	}
}
static int Root_TryShiftCnt(calcOperand_t *co, sint shiftCnt, uint exponent)
{
	calcOperand_t *tmp;
	calcOperand_t *tmp2;
	int ret;

	tmp = calcFromInt(1);
	tmp->E = shiftCnt;
	tmp2 = calcPower(tmp, exponent);

	ret = 0 <= calcComp(co, tmp2);

	ReleaseOperand(tmp);
	ReleaseOperand(tmp2);
	return ret;
}
static sint Root_GetShiftCntMin(calcOperand_t *co, uint exponent)
{
	sint shiftCnt;
	sint ss;

	for (shiftCnt = -1; Root_TryShiftCnt(co, shiftCnt, exponent); shiftCnt *= 2);

	shiftCnt /= 2;

	for (ss = shiftCnt; ss /= 2; )
		if (Root_TryShiftCnt(co, shiftCnt + ss, exponent))
			shiftCnt += ss;

	return shiftCnt;
}
calcOperand_t *calcRoot(calcOperand_t *co, uint exponent)
{
	calcOperand_t *ans;
	sint shiftCnt;

	errorCase(!co);
	errorCase(exponent < 2);
	CheckVars();

	co = GetClone(co);
	co->Sign = 1;
	Trim(co);
	ans = CreateOperand();

	for (shiftCnt = Root_GetShiftCntMin(co, exponent); shiftCnt <= calcBasement; shiftCnt++)
	{
		for (; ; )
		{
			calcOperand_t *tmpAdd;
			calcOperand_t *ansTmp;
			calcOperand_t *tmp2;
			int ret;

			tmpAdd = calcFromInt(1);
			tmpAdd->E = shiftCnt;
			ansTmp = calcAdd(ans, tmpAdd);
			ReleaseOperand(tmpAdd);
			tmp2 = calcPower(ansTmp, exponent);

			ret = calcComp(co, tmp2);

			ReleaseOperand(tmp2);

			if (ret < 0)
			{
				ReleaseOperand(ansTmp);
				break;
			}
			ReleaseOperand(ans);
			ans = ansTmp;

			if (ret == 0)
				goto endSftCtLoop;
		}
	}
endSftCtLoop:
	return ans;
}

// _x
calcOperand_t *calcFromString_x(char *str)
{
	calcOperand_t *ret = calcFromString(str);
	memFree(str);
	return ret;
}
char *calcGetString_x(calcOperand_t *co)
{
	char *ret = calcGetString(co);
	ReleaseOperand(co);
	return ret;
}
char *calc_xc(char *str1, int op, char *str2)
{
	char *ret = calc(str1, op, str2);
	memFree(str1);
	return ret;
}
char *calc_xx(char *str1, int op, char *str2)
{
	char *ret = calc_xc(str1, op, str2);
	memFree(str2);
	return ret;
}
char *calc_cx(char *str1, int op, char *str2)
{
	char *ret = calc(str1, op, str2);
	memFree(str2);
	return ret;
}

// c_
char *c_calcGetString(calcOperand_t *co)
{
	static char *stock;
	memFree(stock);
	return stock = calcGetString(co);
}
char *c_calcGetString_x(calcOperand_t *co)
{
	char *ret = c_calcGetString(co);
	ReleaseOperand(co);
	return ret;
}
char *c_calc(char *str1, int op, char *str2)
{
	static char *stock;
	memFree(stock);
	return stock = calc(str1, op, str2);
}
char *c_calc_xc(char *str1, int op, char *str2)
{
	char *ret = c_calc(str1, op, str2);
	memFree(str1);
	return ret;
}
char *c_calc_xx(char *str1, int op, char *str2)
{
	char *ret = c_calc_xc(str1, op, str2);
	memFree(str2);
	return ret;
}
char *c_calc_cx(char *str1, int op, char *str2)
{
	char *ret = c_calc(str1, op, str2);
	memFree(str2);
	return ret;
}
