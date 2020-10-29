#pragma once

#include "C:\Factory\Common\all.h"

typedef struct calcOperand_st // static member
{
	autoBlock_t *Figures;
	uint DecIndex;
	sint Sign; // 1 or -1, "0" ÇÃÇ∆Ç´ÇÕ 1 Ç∆Ç∑ÇÈÅB
}
calcOperand_t;

extern uint calcRadix;
extern uint calcBasement;
extern uint calcBracketedDecimalMin;

int calcLastMarume;

calcOperand_t *addCalc(calcOperand_t *op1, calcOperand_t *op2);
calcOperand_t *subCalc(calcOperand_t *op1, calcOperand_t *op2);
calcOperand_t *mulCalc(calcOperand_t *op1, calcOperand_t *op2);
calcOperand_t *divCalc(calcOperand_t *op1, calcOperand_t *op2);
sint compCalc(calcOperand_t *op1, calcOperand_t *op2);

calcOperand_t *makeCalcOperand(char *line);
char *makeLineCalcOperand(calcOperand_t *op);
calcOperand_t *copyCalcOperand(calcOperand_t *op);
void releaseCalcOperand(calcOperand_t *op);

char *calcLine(char *line1, int operator, char *line2, uint radix, uint basement);
char *trimCalcLine(char *line, uint radix);
sint compCalcLine(char *line1, char *line2, uint radix);
char *changeRadixCalcLine(char *line, uint radix, uint newRadix, uint basement);

char *calcPower(char *line, uint exponent, uint radix);
char *calcRootPower(char *line, uint exponent, uint radix, uint basement);
uint calcLogarithm(char *line1, char *line2, uint radix);

char *calcLineToMarume(char *line, uint basement);

// _x
char *calcLine_cx(char *line1, int operator, char *line2, uint radix, uint basement);
char *calcLine_xc(char *line1, int operator, char *line2, uint radix, uint basement);
char *calcLine_xx(char *line1, int operator, char *line2, uint radix, uint basement);
char *changeRadixCalcLine_x(char *line, uint radix, uint newRadix, uint basement);
char *calcPower_x(char *line, uint exponent, uint radix);
char *calcRootPower_x(char *line, uint exponent, uint radix, uint basement);
char *calcLineToMarume_x(char *line, uint basement);
