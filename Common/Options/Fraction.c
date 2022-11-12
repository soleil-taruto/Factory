#include "Fraction.h"

static void CheckFraction(Fraction_t *i)
{
	errorCase(!i);
	errorCase(SINTMAX < i->Numer);
	errorCase(SINTMAX < i->Denom && !i->Denom);
	errorCase(i->Sign != 1 && i->Sign != -1);
}

Fraction_t *Frct_Create(uint numer, uint denom, int sign)
{
	Fraction_t *i = memAlloc(sizeof(Fraction_t));

	i->Numer = numer;
	i->Denom = denom;
	i->Sign = sign;

	CheckFraction(i);

	return i;
}
void Frct_Release(Fraction_t *i)
{
	CheckFraction(i);

	memFree(i);
}

static uint64 N;
static uint64 D;

static void Reduction(void)
{
	uint64 n_dest[64];
	uint64 d_dest[64];
	uint ni = 0;
	uint di = 0;

	if (N == 0)
	{
		N = 0;
		D = 1;
		return;
	}
	errorCase(D == 0);

	Factorization(N, n_dest);
	Factorization(D, d_dest);

	for (; ; )
	{
		uint nc = n_dest[ni];
		uint dc = d_dest[di];

		if (!nc || !dc)
			break;

		if (nc == dc)
		{
			n_dest[ni] = 1;
			d_dest[di] = 1;

			ni++;
			di++;
		}
		else if (nc < dc)
		{
			ni++;
		}
		else
		{
			di++;
		}
	}
	N = n_dest[0];
	D = d_dest[0];

	errorCase(N == 0); // 2bs
	errorCase(D == 0); // 2bs

	for (ni = 1; n_dest[ni]; ni++)
		N *= n_dest[ni];

	for (di = 1; d_dest[di]; di++)
		D *= d_dest[di];

	errorCase(N == 0); // 2bs
	errorCase(D == 0); // 2bs
}
static void Normalize(void)
{
	Reduction();

	while (SINTMAX < N || SINTMAX < D) // 丸め！
	{
		cout("< %I64u / %I64u\n", N, D);

		N >>= 1;
		D >>= 1;

		if (D == 0)
		{
			cout("Warning: 分母がゼロになりました。正規化を中止します。\n");

			N = SINTMAX;
			D = 1;
		}
		cout("> %I64u / %I64u\n", N, D);

		Reduction();
	}
}

Fraction_t *Frct_Add(Fraction_t *a, Fraction_t *b)
{
	Fraction_t *ans;

	CheckFraction(a);
	CheckFraction(b);

	if (a->Sign == -1)
	{
		a->Sign = 1;
		ans = Frct_Sub(b, a);
		a->Sign = -1;

		return ans;
	}
	if (b->Sign == -1)
	{
		b->Sign = 1;
		ans = Frct_Sub(a, b);
		b->Sign = -1;

		return ans;
	}
	N = (uint64)a->Numer * (uint64)b->Denom + (uint64)b->Numer * (uint64)a->Denom;
	D = (uint64)a->Denom * (uint64)b->Denom;

	Normalize();

	return Frct_Create(N, D, 1);
}
Fraction_t *Frct_Sub(Fraction_t *a, Fraction_t *b)
{
	Fraction_t *ans;
	uint64 n1;
	uint64 n2;
	int sign;

	CheckFraction(a);
	CheckFraction(b);

	if (a->Sign == -1)
	{
		a->Sign = 1;
		ans = Frct_Add(a, b);
		ans->Sign *= -1;
		a->Sign = -1;

		return ans;
	}
	if (b->Sign == -1)
	{
		b->Sign = 1;
		ans = Frct_Add(a, b);
		b->Sign = -1;

		return ans;
	}
	n1 = (uint64)a->Numer * (uint64)b->Denom;
	n2 = (uint64)b->Numer * (uint64)a->Denom;

	if (n1 < n2)
	{
		N = n2 - n1;
		sign = -1;
	}
	else
	{
		N = n1 - n2;
		sign = 1;
	}
	D = (uint64)a->Denom * (uint64)b->Denom;

	Normalize();

	return Frct_Create(N, D, sign);
}
Fraction_t *Frct_Mul(Fraction_t *a, Fraction_t *b)
{
	CheckFraction(a);
	CheckFraction(b);

	N = (uint64)a->Numer * (uint64)b->Numer;
	D = (uint64)a->Denom * (uint64)b->Denom;

	Normalize();

	return Frct_Create(N, D, a->Sign * b->Sign);
}
Fraction_t *Frct_Div(Fraction_t *a, Fraction_t *b)
{
	CheckFraction(a);
	CheckFraction(b);

	N = (uint64)a->Numer * (uint64)b->Denom;
	D = (uint64)a->Denom * (uint64)b->Numer;

	Normalize();

	return Frct_Create(N, D, a->Sign * b->Sign);
}
Fraction_t *Frct_Calc(Fraction_t *a, int operator, Fraction_t *b)
{
	CheckFraction(a);
	CheckFraction(b);

	switch (operator)
	{
	case '+':
		return Frct_Add(a, b);
	case '-':
		return Frct_Sub(a, b);
	case '*':
		return Frct_Mul(a, b);
	case '/':
		return Frct_Div(a, b);
	}
	error(); // ? unknown operator
	return NULL; // dummy
}
Fraction_t *Frct_Calc_xx(Fraction_t *a, int operator, Fraction_t *b)
{
	Fraction_t *ans = Frct_Calc(a, operator, b);

	Frct_Release(a);
	Frct_Release(b);
	return ans;
}
Fraction_t *Frct_Calc_xc(Fraction_t *a, int operator, Fraction_t *b)
{
	Fraction_t *ans = Frct_Calc(a, operator, b);

	Frct_Release(a);
	return ans;
}
Fraction_t *Frct_Calc_cx(Fraction_t *a, int operator, Fraction_t *b)
{
	Fraction_t *ans = Frct_Calc(a, operator, b);

	Frct_Release(b);
	return ans;
}

static int GV_Sign;

static uint64 GetValue(char *line)
{
	uint64 value = 0;
	char *p;

	if (!*line)
		cout("Warning: 空文字列です。\n");

	GV_Sign = 1;

	for (p = line; *p; p++)
	{
		if (*p == '-')
		{
			GV_Sign = -1;
		}
		else if (*p == '.')
		{
			cout("Warning: 分子又は分母の小数点は無視します。\n");
		}
		else if (m_isdecimal(*p))
		{
			uint n_val = *p - '0';

			if ((UINT64MAX - n_val) / 10 < value)
			{
				cout("Warning: 分子又は分母が最大値を超えました。パースを中止します。[GV]\n");
				break;
			}
			value *= 10;
			value += n_val;
		}
	}
	return value;
}
static Fraction_t *Frct_FromFractionLine(char *line)
{
	autoList_t *tokens = tokenize(line, '/');
	int sign;

	N = GetValue(refLine(tokens, 0));
	sign = GV_Sign;
	D = GetValue(refLine(tokens, 1));
	sign *= GV_Sign;

	Normalize();

	return Frct_Create(N, D, sign);
}
Fraction_t *Frct_FromLine(char *line)
{
	int sign = 1;
	uint d_mul = 1;
	char *p;

	errorCase(!line);

	if (strchr(line, '/')) // ? 分数
		return Frct_FromFractionLine(line);

	N = 0;
	D = 1;

	for (p = line; *p; p++)
	{
		if (*p == '-')
		{
			sign = -1;
		}
		else if (*p == '.')
		{
			d_mul = 10;
		}
		else if (m_isdecimal(*p))
		{
			uint n_val = *p - '0';
			uint c;

			if ((UINT64MAX - n_val) / 10 < N || UINT64MAX / d_mul < D)
			{
				Reduction();

				if ((UINT64MAX - n_val) / 10 < N || UINT64MAX / d_mul < D)
				{
					cout("Warning: 分子又は分母が最大値を超えました。パースを中止します。[FL]\n");
					break;
				}
			}
			N *= 10;
			N += n_val;
			D *= d_mul;
		}
	}

	Normalize();

	return Frct_Create(N, D, sign);
}
char *Frct_ToLine(Fraction_t *i, uint basement)
{
	char *ans = strx("");
	uint64 v;
	uint64 m;
	uint rank;

	CheckFraction(i);
	errorCase(IMAX < basement); // 適当な上限

	if (i->Sign == -1)
		ans = addChar(ans, '-');

	N = i->Numer;
	D = i->Denom;
	v = N / D;
	m = v * D;
	N -= m;

	ans = addLine_x(ans, xcout("%I64u", v));

	for (rank = 0; rank < basement; )
	{
		if (N == 0)
			break;

		if (!rank)
			ans = addChar(ans, '.');

		if (rank + 5 <= basement) // 高速, 5 桁
		{
			N *= 100000;
			v = N / D;
			m = v * D;
			N -= m;

			errorCase(!m_isRange(v, 0, 99999));

			ans = addLine_x(ans, xcout("%05I64u", v));

			rank += 5;
		}
		else
		{
			N *= 10;
			v = N / D;
			m = v * D;
			N -= m;

			errorCase(!m_isRange(v, 0, 9));

			ans = addChar(ans, (int)(v + '0'));

			rank++;
		}
	}
	if (N == 0) // ? 割り切れた。
	{
		if (rank)
		{
			char *p = strchr(ans, '\0');

			while (p[-1] == '0')
				p--;

			*p = '\0';
		}
	}
	else // ? 割り切れない。
	{
		if (!rank)
			ans = addChar(ans, '.');

		ans = addChar(ans, '*');
	}
	return ans;
}
char *Frct_ToFractionLine(Fraction_t *i)
{
	CheckFraction(i);

	return xcout("%s%u/%u", i->Sign == -1 ? "-" : "", i->Numer, i->Denom);
}
Fraction_t *Frct_FromLine_x(char *line)
{
	Fraction_t *ans = Frct_FromLine(line);

	memFree(line);
	return ans;
}
char *Frct_ToLine_x(Fraction_t *i, uint basement)
{
	char *line = Frct_ToLine(i, basement);

	Frct_Release(i);
	return line;
}
char *Frct_ToFractionLine_x(Fraction_t *i)
{
	char *line = Frct_ToFractionLine(i);

	Frct_Release(i);
	return line;
}
