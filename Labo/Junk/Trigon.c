/*
	Trigon.exe [/B BASEMENT]? [ACCUR]?

		BASEMENT -> 0, 1, 2...
		ACCUR    -> 0, 1, 2...
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Calc.h"

#define DEF_ACCUR 9
#define DEF_BASEMENT 25

static uint Basement = DEF_BASEMENT;

static autoList_t *RateList;
static autoList_t *AngleList;
static autoList_t *SineList;

static char *GetCosine(char *rate)
{
	char *iRate = calcLine("1", '-', rate, 10, 0);
	uint index;

	index = findLine(RateList, iRate);
	errorCase(index == getCount(RateList));
	memFree(iRate);
	return getLine(SineList, index);
}
static char *GetMidSine(char *sine1, char *sine2, char *rate1, char *rate2)
{
	char *cosine1 = GetCosine(rate1);
	char *cosine2 = GetCosine(rate2);
	char *midSine;
	char *midCosine;
	char *rRate;

	midSine   = calcLine_xc(calcLine(sine1,   '+', sine2,   10, 0), '*', "0.5", 10, 0);
	midCosine = calcLine_xc(calcLine(cosine1, '+', cosine2, 10, 0), '*', "0.5", 10, 0);

	rRate = calcLine_xx(calcPower(midSine, 2, 10), '+', calcPower(midCosine, 2, 10), 10, 0);
	rRate = calcRootPower_x(rRate, 2, 10, Basement);

	midSine   = calcLine_xc(midSine,   '/', rRate, 10, Basement);
//	midCosine = calcLine_xc(midCosine, '/', rRate, 10, Basement);

	if (compCalcLine(sine2, midSine, 10) == -1) // ? sine2 < midSine ... ŠÛ‚ßŒë·‚É‚æ‚èŽŸ(ã)‚Ì’l‚ðã‰ñ‚Á‚Ä‚µ‚Ü‚Á‚½B
	{
		memFree(midSine);
		midSine = strx(sine2);
	}
	memFree(midCosine);
	memFree(rRate);
	return midSine;
}
static void ExpandOnce(uint index)
{
	char *rate1 = getLine(RateList, index - 1);
	char *rate2 = getLine(RateList, index);
	char *angle1 = getLine(AngleList, index - 1);
	char *angle2 = getLine(AngleList, index);
	char *sine1 = getLine(SineList, index - 1);
	char *sine2 = getLine(SineList, index);
	char *midRate;
	char *midAngle;
	char *midSine;

	midRate  = calcLine_xc(calcLine(rate1,  '+', rate2,  10, 0), '*', "0.5", 10, 0);
	midAngle = calcLine_xc(calcLine(angle1, '+', angle2, 10, 0), '*', "0.5", 10, 0);
	midSine  = GetMidSine(sine1, sine2, rate1, rate2);

	insertElement(RateList,  index, (uint)midRate);
	insertElement(AngleList, index, (uint)midAngle);
	insertElement(SineList,  index, (uint)midSine);
}
static void Expand(uint prn_accur)
{
	uint index;

	for (index = getCount(RateList) - 1; index; index--)
	{
		if (eqIntPulseSec(2, NULL))
		{
			cmdTitle_x(xcout("Trigon - %u %u", prn_accur, index));
		}
		ExpandOnce(index);
	}
}
static void ShootingStar(autoList_t *lines)
{
	while (strchr(getLine(lines, 0), '*'))
	{
		char *line;
		uint index;
		uint sndxmax = 0;

		foreach (lines, line, index)
		{
			char *p = strchr(line, '*');
			uint sndx;

			errorCase(!p);
			sndx = (uint)p - (uint)line;
			sndxmax = m_max(sndxmax, sndx);
		}
		foreach (lines, line, index)
		{
			uint sndx = (uint)strchr(line, '*') - (uint)line;

			while (sndx < sndxmax)
			{
				line = insertChar(line, sndx, ' ');
				sndx++;
			}
			line[sndx] = ' ';
			setElement(lines, index, (uint)line);
		}
	}
}
static void DispAngles(void)
{
	char *rate;
	char *angle;
	char *sine;
	char *cosine;
	uint index;
	autoList_t *lines = newList();
	char *line;

	addElement(lines, (uint)strx("RATE*ANGLE*SINE*COSINE"));

	foreach (RateList, rate, index)
	{
		angle  = getLine(AngleList, index);
		sine   = getLine(SineList, index);
		cosine = getLine(SineList, getCount(SineList) - 1 - index);

		addElement(lines, (uint)xcout("%s*%s*%s*%s", rate, angle, sine, cosine));
	}
	ShootingStar(lines);

	foreach (lines, line, index)
	{
		cout("%s\n", line);
	}
	releaseDim(lines, 1);
}
static void Trigon(uint accur)
{
	RateList = newList();
	AngleList = newList();
	SineList = newList();

	addElement(RateList, (uint)strx("0"));
	addElement(RateList, (uint)strx("1"));
	addElement(AngleList, (uint)strx("0"));
	addElement(AngleList, (uint)strx("90"));
	addElement(SineList, (uint)strx("0"));
	addElement(SineList, (uint)strx("1"));

	while (accur)
	{
		Expand(accur);
		accur--;
	}
	cmdTitle("Trigon");
	DispAngles();

	releaseDim(RateList, 1);
	releaseDim(AngleList, 1);
	releaseDim(SineList, 1);
}
int main(int argc, char **argv)
{
	if (argIs("/B"))
	{
		Basement = toValue(nextArg());
	}

	if (hasArgs(1))
	{
		Trigon(toValue(nextArg()));
		return;
	}
	Trigon(DEF_ACCUR);
}
