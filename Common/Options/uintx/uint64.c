#include "uint64.h"

UI64_t ToUI64(uint src[2])
{
	UI64_t ans;

	ans.L = ToUI32(src + 0);
	ans.H = ToUI32(src + 1);

	return ans;
}
UI64_t UI64_x(uint x)
{
	UI64_t ans;

	ans.L = UI32_x(x);
	ans.H = UI32_0();

	return ans;
}
UI64_t UI64_0(void)
{
	UI64_t ans;

	ans.L = UI32_0();
	ans.H = UI32_0();

	return ans;
}
void FromUI64(UI64_t a, uint dest[2])
{
	FromUI32(a.L, dest + 0);
	FromUI32(a.H, dest + 1);
}
uint UI64_y(UI64_t a)
{
	return UI32_y(a.L);
}

UI64_t UI64_Inv(UI64_t a)
{
	UI64_t ans;

	ans.L = UI32_Inv(a.L);
	ans.H = UI32_Inv(a.H);

	return ans;
}
UI64_t UI64_Add(UI64_t a, UI64_t b, UI64_t ans[2])
{
	UI32_t c[2];
	UI32_t d[2];
	UI32_t e[2];
	UI32_t f[2];
	UI64_t tmp[2];

	if (!ans)
		ans = tmp;

	UI32_Add(a.L, b.L, c);
	UI32_Add(a.H, b.H, d);
	UI32_Add(d[0], c[1], e);
	UI32_Add(d[1], e[1], f);

	ans[0].L = c[0];
	ans[0].H = e[0];
	ans[1].L = f[0];
	ans[1].H = UI32_0();

	return ans[0];
}
UI64_t UI64_Sub(UI64_t a, UI64_t b)
{
	UI64_t tmp[2];

	b = UI64_Inv(b);
	b = UI64_Add(b, UI64_x(1), tmp);

	return UI64_Add(a, b, tmp);
}
UI64_t UI64_Mul(UI64_t a, UI64_t b, UI64_t ans[2])
{
	UI32_t c[2];
	UI32_t d[2];
	UI32_t e[2];
	UI32_t f[2];
	UI32_t m[5][5];
	uint mc[5] = { 0 }; // max are 1, 3, 5, 5, 4
	uint i;
	UI64_t tmp[2];

	if (!ans)
		ans = tmp;

	UI32_Mul(a.L, b.L, c);
	UI32_Mul(a.L, b.H, d);
	UI32_Mul(a.H, b.L, e);
	UI32_Mul(a.H, b.H, f);

	m[0][mc[0]++] = c[0];
	m[1][mc[1]++] = c[1];
	m[1][mc[1]++] = d[0];
	m[2][mc[2]++] = d[1];
	m[1][mc[1]++] = e[0];
	m[2][mc[2]++] = e[1];
	m[2][mc[2]++] = f[0];
	m[3][mc[3]++] = f[1];

	for (i = 1; i < 4; i++)
	while (2 <= mc[i])
	{
		UI32_t t = m[i][--mc[i]];
		UI32_t u = m[i][--mc[i]];
		UI32_t v[2];

		m[i][mc[i]++] = UI32_Add(t, u, v);
		m[i + 1][mc[i + 1]++] = v[1];
	}

	ans[0].L = m[0][0];
	ans[0].H = m[1][0];
	ans[1].L = m[2][0];
	ans[1].H = m[3][0];

	return ans[0];
}
static UI64_t DivSub(UI64_t a, UI64_t b, UI64_t ans[2], uint reti)
{
	UI64_t tmp[2];

	errorCase(UI64_IsZero(b));

	if (!ans)
		ans = tmp;

	ans[0] = UI64_0();

	if (UI32_IsZero(b.H))
	{
		{
			UI64_t dd;

			dd.L = UI32_0();
			dd.H = UI32_Div(a.H, b.L, NULL);

			ans[0] = UI64_Add(ans[0], dd, NULL);
			a = UI64_Sub(a, UI64_Mul(b, dd, NULL));
		}

		while (!UI32_IsZero(a.H))
		{
			UI64_t dd;
			UI32_t d2[2];

			UI32_Mul(UI32_Div(UI32_Fill(), b.L, NULL), a.H, d2);

			dd.L = d2[0];
			dd.H = d2[1];

			ans[0] = UI64_Add(ans[0], dd, NULL);
			a = UI64_Sub(a, UI64_Mul(b, dd, NULL));
		}

		{
			UI64_t dd;

			dd.L = UI32_Div(a.L, b.L, NULL);
			dd.H = UI32_0();

			ans[0] = UI64_Add(ans[0], dd, NULL);
			a = UI64_Sub(a, UI64_Mul(b, dd, NULL));
		}
	}
	else
	{
		if (!UI32_IsFill(b.H))
		{
			UI32_t c = UI32_Add(b.H, UI32_x(1), NULL);

			for (; ; )
			{
				UI32_t d = UI32_Div(a.H, c, NULL);
				UI64_t dd;

				if (UI32_IsZero(d))
					break;

				dd.L = d;
				dd.H = UI32_0();

				ans[0] = UI64_Add(ans[0], dd, NULL);
				a = UI64_Sub(a, UI64_Mul(b, dd, NULL));
			}
		}
		while (0 <= UI64_Comp(a, b))
		{
			ans[0] = UI64_Add(ans[0], UI64_x(1), NULL);
			a = UI64_Sub(a, b);
		}
	}
	ans[1] = a;
	return ans[reti];
}
UI64_t UI64_Div(UI64_t a, UI64_t b, UI64_t ans[2])
{
	return DivSub(a, b, ans, 0);
}
UI64_t UI64_Mod(UI64_t a, UI64_t b, UI64_t ans[2])
{
	return DivSub(a, b, ans, 1);
}

int UI64_IsZero(UI64_t a)
{
	return UI32_IsZero(a.L) && UI32_IsZero(a.H);
}
int UI64_IsFill(UI64_t a)
{
	return UI32_IsFill(a.L) && UI32_IsFill(a.H);
}
sint UI64_Comp(UI64_t a, UI64_t b)
{
	sint ret = UI32_Comp(a.H, b.H);

	if (!ret)
		ret = UI32_Comp(a.L, b.L);

	return ret;
}
UI64_t UI64_Fill(void)
{
	UI64_t ans;

	ans.L = UI32_Fill();
	ans.H = UI32_Fill();

	return ans;
}

UI64_t UI64_DivTwo_Rem(UI64_t a, uint *rem)
{
	UI64_t ans;

	ans.H = UI32_DivTwo_Rem(a.H, rem);
	ans.L = UI32_DivTwo_Rem(a.L, rem);

	return ans;
}
UI64_t UI64_DivTwo(UI64_t a)
{
	uint rem = 0;

	return UI64_DivTwo_Rem(a, &rem);
}
