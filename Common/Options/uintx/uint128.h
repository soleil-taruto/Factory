#pragma once

#include "C:\Factory\Common\all.h"
#include "uint64.h"

typedef struct UI128_st
{
	UI64_t L;
	UI64_t H;
}
UI128_t;

UI128_t ToUI128(uint src[4]);
UI128_t UI128_x(uint x);
UI128_t UI128_0(void);
void FromUI128(UI128_t a, uint dest[4]);
uint UI128_y(UI128_t a);

UI128_t UI128_Inv(UI128_t a);
UI128_t UI128_Add(UI128_t a, UI128_t b, UI128_t ans[2]);
UI128_t UI128_Sub(UI128_t a, UI128_t b);
UI128_t UI128_Mul(UI128_t a, UI128_t b, UI128_t ans[2]);
UI128_t UI128_Div(UI128_t a, UI128_t b, UI128_t ans[2]);
UI128_t UI128_Mod(UI128_t a, UI128_t b, UI128_t ans[2]);

int UI128_IsZero(UI128_t a);
int UI128_IsFill(UI128_t a);
sint UI128_Comp(UI128_t a, UI128_t b);
UI128_t UI128_Fill(void);

UI128_t UI128_DivTwo_Rem(UI128_t a, uint *rem);
UI128_t UI128_DivTwo(UI128_t a);
