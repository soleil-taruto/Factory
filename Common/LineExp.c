/*
	パターンマッチ

	<RANGES>         - 0 ～ UINTMAX
	<num,RANGES>     - num ～ num
	<min,,RANGES>    - min ～ UINTMAX
	<,max,RANGES>    - 0 ～ max
	<min,max,RANGES> - min ～ max (min <= max)

	RANGES が空文字列のときは全マッチ (\x01\xffと同じ) ex. <10,> ... 10バイトのあらゆる文字列

	エスケープ

	<> の外のみ
		"//"  ->  "/"
		"/<"  ->  "<"

	RANGES 内のみ
		"//"  ->  "/"
		"/G"  ->  ">"
		"/M"  ->  ","
*/

#include "all.h"

static int GetRngChar(char **pp)
{
	int chr = **pp;

	errorCase(!chr);

	if (chr == '/')
	{
		++*pp;
		chr = **pp;

//		errorCase(!chr);
		errorCase(chr != '/' && chr != 'G' && chr != 'M');

		     if (chr == 'G') chr = '>';
		else if (chr == 'M') chr = ',';
	}
	++*pp;
	return chr;
}
static int IsMatch(int chr, char *rngs)
{
	char *p;

	if (*rngs == '\0') // rngs == "" -> 全マッチ
		return 1;

	for (p = rngs; *p; )
	{
		int min;
		int max;

		min = GetRngChar(&p);
		max = GetRngChar(&p);

		errorCase(max < min);

		if (min <= chr && chr <= max)
		{
			return 1;
		}
	}
	return 0;
}
int lineExp(char *format, char *line) // ret: ? マッチした。
{
	char *fp = format;
	char *lp = line;
	char *p;
	char *tmpl;
	autoList_t *lnums;
	uint nummin;
	uint nummax;
	char *rngs = NULL;
	uint i;
	int retval = 0;

	for (; ; fp++, lp++)
	{
		if (*fp == '\0')
		{
			if (*lp)
				goto nomatch;

			break;
		}
		if (*fp == '/')
		{
			fp++;
			errorCase(*fp != '/' && *fp != '<');

			if (*fp != *lp)
			{
				goto nomatch;
			}
		}
		else if (*fp == '<')
		{
			fp++;
			p = strchr(fp, '>');
			errorCase(!p);

			tmpl = strxl(fp, (uint)p - (uint)fp);
			lnums = tokenize(tmpl, ',');

			if (getCount(lnums) == 1)
				insertElement(lnums, 0, (uint)strx("")); // <r> to <,r>

			if (getCount(lnums) == 2)
				insertElement(lnums, 1, (uint)strx(getLine(lnums, 0))); // <n,r> to <n,n,r>

			errorCase(getCount(lnums) != 3);

			nummin = toValue(getLine(lnums, 0));
			nummax = toValue(getLine(lnums, 1));
			rngs = strx(getLine(lnums, 2));

			if (!nummax)
				nummax = UINTMAX;

			errorCase(nummax < nummin);

			memFree(tmpl);
			releaseDim(lnums, 1);

			fp = p + 1;

			for (i = 0; ; i++)
			{
				if (nummin <= i)
					if (lineExp(fp, lp))
						break;

				if (*lp == '\0' || !IsMatch(*lp, rngs))
					goto nomatch;

				if (nummax <= i)
					goto nomatch;

				lp++;
			}
			break;
		}
		else if (*fp != *lp)
		{
			goto nomatch;
		}
	}
	retval = 1;

nomatch:
	memFree(rngs);
	return retval;
}
int lineExpICase(char *format, char *line)
{
	char *tmp1 = strx(format);
	char *tmp2 = strx(line);
	int ret;

	toLowerLine(tmp1);
	toLowerLine(tmp2);

	ret = lineExp(tmp1, tmp2);

	memFree(tmp1);
	memFree(tmp2);
	return ret;
}
