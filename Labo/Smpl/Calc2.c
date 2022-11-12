/*
	Calc2.exe [/R <Radix>] [/E <Effect>] /EM <Number>

	Calc2.exe [/R <Radix>] [/S] [/E <Effect>] [/-R] /P <Number> <Exponent>

	Calc2.exe [/R <Radix>] [/B <Basement>] [/S] [/E <Effect>] [/-R] /R <Number> <Exponent>

	Calc2.exe [/R <Radix>] [/B <Basement>] [/-R] <Number> <Operator> <Number> <Effect>

	Calc2.exe [/R <Radix>] [/B <Basement>] [/S] [/E <Effect>] [/-R] <Number> <Operator> <Number>

	Calc2.exe [/R <Radix>] [/-R] <Number> <Effect>

	Calc2.exe [/R <Radix>] [/S] [/E <Effect>] [/-R] <Number>

		Radix    ...     2 Å` 36
		Basement ... -IMAX Å` IMAX
		Effect   ...     0 Å` IMAX
		Exponent ...     0 Å` 4294967295

		Number ... é¿êî | é¿êîÇÃéwêîï\ãL

			ëÂÇ´Ç»éwêîÇéwíËÇ∑ÇÈÇ±Ç∆Ç…ÇÊÇ¡ÇƒÉÅÉÇÉäïsë´Ç‚èàóùÇ…éûä‘Ç™ä|Ç©ÇÈèÛãµÇà¯Ç´ãNÇ±ÇπÇÈÇ±Ç∆Ç…íçà”ÅI

		Operator ... + - * /

	----
	ó·

	Calc2 /S 123.456   ==>   1.23456E+02

	Calc2 /E 10 /EM 123.456   ==>   123.4560000

	- - -

	Calc2 /B 20 2 / 3 21   ==>   6.66666666666666666660E-01

	Calc2 /B 20 2 / 3 20   ==>   6.6666666666666666666E-01

	Calc2 /B 20 2 / 3 19   ==>   6.666666666666666667E-01

	- - -

	Calc2 /B 19 2 / 3 20   ==>   6.6666666666666666660E-01

	Calc2 /B 20 2 / 3 20   ==>   6.6666666666666666666E-01

	Calc2 /B 21 2 / 3 20   ==>   6.6666666666666666667E-01
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Calc2.h"

int main(int argc, char **argv)
{
readArgs:
	if (argIs("/X"))
	{
		calcRadix = toValue(nextArg()); // 2 Å` 36, îÕàÕäO -> calc()Ç≈error();
		goto readArgs;
	}
	if (argIs("/B"))
	{
		calcBasement = atoi(nextArg()); // -IMAX Å` IMAX, îÕàÕäO -> calc()Ç≈error();
		goto readArgs;
	}
	if (argIs("/S"))
	{
		calcScient = 1;
		goto readArgs;
	}
	if (argIs("/E"))
	{
		calcEffect = toValue(nextArg()); // 0 == ñ≥å¯, 1 Å` IMAX, îÕàÕäO -> calc()Ç≈error();
		goto readArgs;
	}
	if (argIs("/-R"))
	{
		calcRndOff = 0;
		goto readArgs;
	}

	/*
		Calc2 /e 10 /em 123.456 Ç∆Ç©
	*/
	if (argIs("/EM"))
	{
		calcOperand_t *co = calcFromString(nextArg());
		char *ret;

		ret = calcGetSmplString_EM(co, calcEffect);
		calcRelease(co);
		cout("%s\n", ret);
		memFree(ret);
		return;
	}

	/*
		Ç◊Ç´èÊ
	*/
	if (argIs("/P"))
	{
		calcOperand_t *co = calcFromString(getArg(0));
		uint exponent = toValue(getArg(1));

		cout("%s\n", c_calcGetString_x(calcPower(co, exponent)));

		calcRelease(co);
		return;
	}

	/*
		Ç◊Ç´ç™
	*/
	if (argIs("/R"))
	{
		calcOperand_t *co = calcFromString(getArg(0));
		uint exponent = toValue(getArg(1));

		cout("%s\n", c_calcGetString_x(calcRoot(co, exponent)));

		calcRelease(co);
		return;
	}

	if (hasArgs(4))
	{
		calcScient = 1;
		calcEffect = toValue(getArg(3));
		cout("%s\n", c_calc(getArg(0), getArg(1)[0], getArg(2)));
		return;
	}
	if (hasArgs(3))
	{
		cout("%s\n", c_calc(getArg(0), getArg(1)[0], getArg(2)));
		return;
	}
	if (hasArgs(2))
	{
		calcScient = 1;
		calcEffect = toValue(getArg(1));
		cout("%s\n", c_calcGetString_x(calcFromString(getArg(0))));
		return;
	}
	if (hasArgs(1))
	{
		cout("%s\n", c_calcGetString_x(calcFromString(getArg(0))));
		return;
	}
	error();
}
