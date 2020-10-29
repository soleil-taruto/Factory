#pragma once

#include "C:\Factory\Common\all.h"

typedef struct calcOperand_st // static member
{
	autoBlock_t *F;
	sint E;
	sint Sign;
}
calcOperand_t;

extern uint calcRadix;
extern sint calcBasement;
extern int calcScient;
extern uint calcEffect;
extern int calcRndOff;

calcOperand_t *calcFromSmplString(char *str);
calcOperand_t *calcFromString(char *str);
calcOperand_t *calcFromString_x(char *str);
calcOperand_t *calcFromInt(sint value);
void calcRelease(calcOperand_t *co);

char *calcGetSmplString_EM(calcOperand_t *co, uint effectMin);
char *calcGetSmplString(calcOperand_t *co);
char *calcGetString(calcOperand_t *co);
char *calcGetString_x(calcOperand_t *co);
char *c_calcGetString(calcOperand_t *co);
char *c_calcGetString_x(calcOperand_t *co);

calcOperand_t *calcAdd(calcOperand_t *co1, calcOperand_t *co2);
calcOperand_t *calcSub(calcOperand_t *co1, calcOperand_t *co2);
calcOperand_t *calcMul(calcOperand_t *co1, calcOperand_t *co2);
calcOperand_t *calcDiv(calcOperand_t *co1, calcOperand_t *co2);

char *calc(char *str1, int op, char *str2);
char *calc_xc(char *str1, int op, char *str2);
char *calc_xx(char *str1, int op, char *str2);
char *calc_cx(char *str1, int op, char *str2);

char *c_calc(char *str1, int op, char *str2);
char *c_calc_xc(char *str1, int op, char *str2);
char *c_calc_xx(char *str1, int op, char *str2);
char *c_calc_cx(char *str1, int op, char *str2);

int calcComp(calcOperand_t *co1, calcOperand_t *co2);
calcOperand_t *calcPower(calcOperand_t *co, uint exponent);
calcOperand_t *calcRoot(calcOperand_t *co, uint exponent);
