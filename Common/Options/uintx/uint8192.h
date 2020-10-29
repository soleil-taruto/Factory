#pragma once

#include "C:\Factory\Common\all.h"
#include "uint4096.h"

typedef struct UI8192_st
{
	UI4096_t L;
	UI4096_t H;
}
UI8192_t;

UI8192_t ToUI8192(uint src[256]);
UI8192_t UI8192_x(uint x);
UI8192_t UI8192_0(void);
void FromUI8192(UI8192_t a, uint dest[256]);
uint UI8192_y(UI8192_t a);

UI8192_t UI8192_Inv(UI8192_t a);
UI8192_t UI8192_Add(UI8192_t a, UI8192_t b, UI8192_t ans[2]);
UI8192_t UI8192_Sub(UI8192_t a, UI8192_t b);
UI8192_t UI8192_Mul(UI8192_t a, UI8192_t b, UI8192_t ans[2]);
UI8192_t UI8192_Div(UI8192_t a, UI8192_t b, UI8192_t ans[2]);
UI8192_t UI8192_Mod(UI8192_t a, UI8192_t b, UI8192_t ans[2]);

int UI8192_IsZero(UI8192_t a);
int UI8192_IsFill(UI8192_t a);
sint UI8192_Comp(UI8192_t a, UI8192_t b);
UI8192_t UI8192_Fill(void);

UI8192_t UI8192_DivTwo_Rem(UI8192_t a, uint *rem);
UI8192_t UI8192_DivTwo(UI8192_t a);
