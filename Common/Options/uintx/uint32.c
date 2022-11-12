#include "uint32.h"

UI32_t ToUI32(uint src[1])
{
	UI32_t ans;

	ans.Value = src[0];

	return ans;
}
UI32_t UI32_x(uint x)
{
	UI32_t ans;

	ans.Value = x;

	return ans;
}
UI32_t UI32_0(void)
{
	UI32_t ans;

	ans.Value = 0;

	return ans;
}
void FromUI32(UI32_t a, uint dest[1])
{
	dest[0] = a.Value;
}
uint UI32_y(UI32_t a)
{
	return a.Value;
}

UI32_t UI32_Inv(UI32_t a)
{
	UI32_t ans;

	ans.Value = ~a.Value;

	return ans;
}
UI32_t UI32_Add(UI32_t a, UI32_t b, UI32_t ans[2])
{
	uint64 val = (uint64)a.Value + b.Value;
	UI32_t tmp[2];

	if (!ans)
		ans = tmp;

	ans[0].Value = (uint)(val & 0xffffffffui64);
	ans[1].Value = (uint)(val >> 32);

	return ans[0];
}
UI32_t UI32_Sub(UI32_t a, UI32_t b)
{
	b = UI32_Inv(b);
	b = UI32_Add(b, UI32_x(1), NULL);

	return UI32_Add(a, b, NULL);
}
UI32_t UI32_Mul(UI32_t a, UI32_t b, UI32_t ans[2])
{
	uint64 val = (uint64)a.Value * b.Value;
	UI32_t tmp[2];

	if (!ans)
		ans = tmp;

	ans[0].Value = (uint)(val & 0xffffffffui64);
	ans[1].Value = (uint)(val >> 32);

	return ans[0];
}
UI32_t UI32_Div(UI32_t a, UI32_t b, UI32_t ans[2])
{
	UI32_t tmp[2];

	errorCase(!b.Value);

	if (!ans)
		ans = tmp;

	ans[0].Value = a.Value / b.Value;
	ans[1].Value = a.Value % b.Value;

	return ans[0];
}
UI32_t UI32_Mod(UI32_t a, UI32_t b, UI32_t ans[2])
{
	UI32_t tmp[2];

	errorCase(!b.Value);

	if (!ans)
		ans = tmp;

	ans[0].Value = a.Value / b.Value;
	ans[1].Value = a.Value % b.Value;

	return ans[1];
}

int UI32_IsZero(UI32_t a)
{
	return a.Value == 0;
}
int UI32_IsFill(UI32_t a)
{
	return a.Value == 0xffffffff;
}
sint UI32_Comp(UI32_t a, UI32_t b)
{
	return m_simpleComp(a.Value, b.Value);
}
UI32_t UI32_Fill(void)
{
	UI32_t ans;

	ans.Value = 0xffffffff;

	return ans;
}

UI32_t UI32_DivTwo_Rem(UI32_t a, uint *rem)
{
	uint b = a.Value & 1;

	a.Value >>= 1;
	a.Value |= *rem << 31;

	*rem = b;

	return a;
}
UI32_t UI32_DivTwo(UI32_t a)
{
	a.Value >>= 1;
	return a;
}
