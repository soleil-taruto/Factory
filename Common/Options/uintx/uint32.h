#pragma once

#include "C:\Factory\Common\all.h"

typedef struct UI32_st
{
	uint Value;
}
UI32_t;

UI32_t ToUI32(uint src[1]);
UI32_t UI32_x(uint x);
UI32_t UI32_0(void);
void FromUI32(UI32_t a, uint dest[1]);
uint UI32_y(UI32_t a);

UI32_t UI32_Inv(UI32_t a);
UI32_t UI32_Add(UI32_t a, UI32_t b, UI32_t ans[2]);
UI32_t UI32_Sub(UI32_t a, UI32_t b);
UI32_t UI32_Mul(UI32_t a, UI32_t b, UI32_t ans[2]);
UI32_t UI32_Div(UI32_t a, UI32_t b, UI32_t ans[2]);
UI32_t UI32_Mod(UI32_t a, UI32_t b, UI32_t ans[2]);

int UI32_IsZero(UI32_t a);
int UI32_IsFill(UI32_t a);
sint UI32_Comp(UI32_t a, UI32_t b);
UI32_t UI32_Fill(void);

UI32_t UI32_DivTwo_Rem(UI32_t a, uint *rem);
UI32_t UI32_DivTwo(UI32_t a);
