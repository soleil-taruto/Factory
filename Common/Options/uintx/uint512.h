#pragma once

#include "C:\Factory\Common\all.h"
#include "uint256.h"

typedef struct UI512_st
{
	UI256_t L;
	UI256_t H;
}
UI512_t;

UI512_t ToUI512(uint src[16]);
UI512_t UI512_x(uint x);
UI512_t UI512_0(void);
void FromUI512(UI512_t a, uint dest[16]);
uint UI512_y(UI512_t a);

UI512_t UI512_Inv(UI512_t a);
UI512_t UI512_Add(UI512_t a, UI512_t b, UI512_t ans[2]);
UI512_t UI512_Sub(UI512_t a, UI512_t b);
UI512_t UI512_Mul(UI512_t a, UI512_t b, UI512_t ans[2]);
UI512_t UI512_Div(UI512_t a, UI512_t b, UI512_t ans[2]);
UI512_t UI512_Mod(UI512_t a, UI512_t b, UI512_t ans[2]);

int UI512_IsZero(UI512_t a);
int UI512_IsFill(UI512_t a);
sint UI512_Comp(UI512_t a, UI512_t b);
UI512_t UI512_Fill(void);

UI512_t UI512_DivTwo_Rem(UI512_t a, uint *rem);
UI512_t UI512_DivTwo(UI512_t a);
