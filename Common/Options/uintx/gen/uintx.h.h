#pragma once

#include "C:\Factory\Common\all.h"
#include "uint$hBIT.h"

typedef struct UI$BIT_st
{
	UI$hBIT_t L;
	UI$hBIT_t H;
}
UI$BIT_t;

UI$BIT_t ToUI$BIT(uint src[$SZ]);
UI$BIT_t UI$BIT_x(uint x);
UI$BIT_t UI$BIT_0(void);
void FromUI$BIT(UI$BIT_t a, uint dest[$SZ]);
uint UI$BIT_y(UI$BIT_t a);

UI$BIT_t UI$BIT_Inv(UI$BIT_t a);
UI$BIT_t UI$BIT_Add(UI$BIT_t a, UI$BIT_t b, UI$BIT_t ans[2]);
UI$BIT_t UI$BIT_Sub(UI$BIT_t a, UI$BIT_t b);
UI$BIT_t UI$BIT_Mul(UI$BIT_t a, UI$BIT_t b, UI$BIT_t ans[2]);
UI$BIT_t UI$BIT_Div(UI$BIT_t a, UI$BIT_t b, UI$BIT_t ans[2]);
UI$BIT_t UI$BIT_Mod(UI$BIT_t a, UI$BIT_t b, UI$BIT_t ans[2]);

int UI$BIT_IsZero(UI$BIT_t a);
int UI$BIT_IsFill(UI$BIT_t a);
sint UI$BIT_Comp(UI$BIT_t a, UI$BIT_t b);
UI$BIT_t UI$BIT_Fill(void);

UI$BIT_t UI$BIT_DivTwo_Rem(UI$BIT_t a, uint *rem);
UI$BIT_t UI$BIT_DivTwo(UI$BIT_t a);
