#pragma once

#include "C:\Factory\Common\all.h"
#include "Prime.h"

typedef struct Fraction_st // static member
{
	uint Numer; // 0 Å` SINTMAX
	uint Denom; // 1 Å` SINTMAX
	sint Sign; // 1 or -1
}
Fraction_t;

Fraction_t *Frct_Create(uint numer, uint denom, int sign);
void Frct_Release(Fraction_t *i);

Fraction_t *Frct_Add(Fraction_t *a, Fraction_t *b);
Fraction_t *Frct_Sub(Fraction_t *a, Fraction_t *b);
Fraction_t *Frct_Mul(Fraction_t *a, Fraction_t *b);
Fraction_t *Frct_Div(Fraction_t *a, Fraction_t *b);

Fraction_t *Frct_Calc(Fraction_t *a, int operator, Fraction_t *b);
Fraction_t *Frct_Calc_xx(Fraction_t *a, int operator, Fraction_t *b);
Fraction_t *Frct_Calc_xc(Fraction_t *a, int operator, Fraction_t *b);
Fraction_t *Frct_Calc_cx(Fraction_t *a, int operator, Fraction_t *b);

Fraction_t *Frct_FromLine(char *line);
Fraction_t *Frct_FromLine_x(char *line);
char *Frct_ToLine(Fraction_t *i, uint basement);
char *Frct_ToLine_x(Fraction_t *i, uint basement);
char *Frct_ToFractionLine(Fraction_t *i);
char *Frct_ToFractionLine_x(Fraction_t *i);
