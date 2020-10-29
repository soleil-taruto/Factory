#pragma once

#include "C:\Factory\Common\all.h"
#include "uint1024.h"

typedef struct UI2048_st
{
	UI1024_t L;
	UI1024_t H;
}
UI2048_t;

UI2048_t ToUI2048(uint src[64]);
UI2048_t UI2048_x(uint x);
UI2048_t UI2048_0(void);
void FromUI2048(UI2048_t a, uint dest[64]);
uint UI2048_y(UI2048_t a);

UI2048_t UI2048_Inv(UI2048_t a);
UI2048_t UI2048_Add(UI2048_t a, UI2048_t b, UI2048_t ans[2]);
UI2048_t UI2048_Sub(UI2048_t a, UI2048_t b);
UI2048_t UI2048_Mul(UI2048_t a, UI2048_t b, UI2048_t ans[2]);
UI2048_t UI2048_Div(UI2048_t a, UI2048_t b, UI2048_t ans[2]);
UI2048_t UI2048_Mod(UI2048_t a, UI2048_t b, UI2048_t ans[2]);

int UI2048_IsZero(UI2048_t a);
int UI2048_IsFill(UI2048_t a);
sint UI2048_Comp(UI2048_t a, UI2048_t b);
UI2048_t UI2048_Fill(void);

UI2048_t UI2048_DivTwo_Rem(UI2048_t a, uint *rem);
UI2048_t UI2048_DivTwo(UI2048_t a);
