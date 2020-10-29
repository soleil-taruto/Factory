#pragma once

#include "C:\Factory\Common\all.h"
#include "uint512.h"

typedef struct UI1024_st
{
	UI512_t L;
	UI512_t H;
}
UI1024_t;

UI1024_t ToUI1024(uint src[32]);
UI1024_t UI1024_x(uint x);
UI1024_t UI1024_0(void);
void FromUI1024(UI1024_t a, uint dest[32]);
uint UI1024_y(UI1024_t a);

UI1024_t UI1024_Inv(UI1024_t a);
UI1024_t UI1024_Add(UI1024_t a, UI1024_t b, UI1024_t ans[2]);
UI1024_t UI1024_Sub(UI1024_t a, UI1024_t b);
UI1024_t UI1024_Mul(UI1024_t a, UI1024_t b, UI1024_t ans[2]);
UI1024_t UI1024_Div(UI1024_t a, UI1024_t b, UI1024_t ans[2]);
UI1024_t UI1024_Mod(UI1024_t a, UI1024_t b, UI1024_t ans[2]);

int UI1024_IsZero(UI1024_t a);
int UI1024_IsFill(UI1024_t a);
sint UI1024_Comp(UI1024_t a, UI1024_t b);
UI1024_t UI1024_Fill(void);

UI1024_t UI1024_DivTwo_Rem(UI1024_t a, uint *rem);
UI1024_t UI1024_DivTwo(UI1024_t a);
