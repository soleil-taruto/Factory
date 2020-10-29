#pragma once

#include "C:\Factory\Common\all.h"
#include "uint2048.h"

typedef struct UI4096_st
{
	UI2048_t L;
	UI2048_t H;
}
UI4096_t;

UI4096_t ToUI4096(uint src[128]);
UI4096_t UI4096_x(uint x);
UI4096_t UI4096_0(void);
void FromUI4096(UI4096_t a, uint dest[128]);
uint UI4096_y(UI4096_t a);

UI4096_t UI4096_Inv(UI4096_t a);
UI4096_t UI4096_Add(UI4096_t a, UI4096_t b, UI4096_t ans[2]);
UI4096_t UI4096_Sub(UI4096_t a, UI4096_t b);
UI4096_t UI4096_Mul(UI4096_t a, UI4096_t b, UI4096_t ans[2]);
UI4096_t UI4096_Div(UI4096_t a, UI4096_t b, UI4096_t ans[2]);
UI4096_t UI4096_Mod(UI4096_t a, UI4096_t b, UI4096_t ans[2]);

int UI4096_IsZero(UI4096_t a);
int UI4096_IsFill(UI4096_t a);
sint UI4096_Comp(UI4096_t a, UI4096_t b);
UI4096_t UI4096_Fill(void);

UI4096_t UI4096_DivTwo_Rem(UI4096_t a, uint *rem);
UI4096_t UI4096_DivTwo(UI4096_t a);
