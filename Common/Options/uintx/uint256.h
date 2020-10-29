#pragma once

#include "C:\Factory\Common\all.h"
#include "uint128.h"

typedef struct UI256_st
{
	UI128_t L;
	UI128_t H;
}
UI256_t;

UI256_t ToUI256(uint src[8]);
UI256_t UI256_x(uint x);
UI256_t UI256_0(void);
void FromUI256(UI256_t a, uint dest[8]);
uint UI256_y(UI256_t a);

UI256_t UI256_Inv(UI256_t a);
UI256_t UI256_Add(UI256_t a, UI256_t b, UI256_t ans[2]);
UI256_t UI256_Sub(UI256_t a, UI256_t b);
UI256_t UI256_Mul(UI256_t a, UI256_t b, UI256_t ans[2]);
UI256_t UI256_Div(UI256_t a, UI256_t b, UI256_t ans[2]);
UI256_t UI256_Mod(UI256_t a, UI256_t b, UI256_t ans[2]);

int UI256_IsZero(UI256_t a);
int UI256_IsFill(UI256_t a);
sint UI256_Comp(UI256_t a, UI256_t b);
UI256_t UI256_Fill(void);

UI256_t UI256_DivTwo_Rem(UI256_t a, uint *rem);
UI256_t UI256_DivTwo(UI256_t a);
