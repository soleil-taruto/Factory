#pragma once

#include "C:\Factory\Common\all.h"
#include "uint32.h"

typedef struct UI64_st
{
	UI32_t L;
	UI32_t H;
}
UI64_t;

UI64_t ToUI64(uint src[2]);
UI64_t UI64_x(uint x);
UI64_t UI64_0(void);
void FromUI64(UI64_t a, uint dest[2]);
uint UI64_y(UI64_t a);

UI64_t UI64_Inv(UI64_t a);
UI64_t UI64_Add(UI64_t a, UI64_t b, UI64_t ans[2]);
UI64_t UI64_Sub(UI64_t a, UI64_t b);
UI64_t UI64_Mul(UI64_t a, UI64_t b, UI64_t ans[2]);
UI64_t UI64_Div(UI64_t a, UI64_t b, UI64_t ans[2]);
UI64_t UI64_Mod(UI64_t a, UI64_t b, UI64_t ans[2]);

int UI64_IsZero(UI64_t a);
int UI64_IsFill(UI64_t a);
sint UI64_Comp(UI64_t a, UI64_t b);
UI64_t UI64_Fill(void);

UI64_t UI64_DivTwo_Rem(UI64_t a, uint *rem);
UI64_t UI64_DivTwo(UI64_t a);
