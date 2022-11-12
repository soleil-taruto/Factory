#include "uint128.h"

UI128_t ToUI128(uint src[4])
{
	UI128_t ans;

	ans.L = ToUI64(src + 0);
	ans.H = ToUI64(src + 2);

	return ans;
}
UI128_t UI128_x(uint x)
{
	UI128_t ans;

	ans.L = UI64_x(x);
	ans.H = UI64_0();

	return ans;
}
UI128_t UI128_0(void)
{
	UI128_t ans;

	ans.L = UI64_0();
	ans.H = UI64_0();

	return ans;
}
void FromUI128(UI128_t a, uint dest[4])
{
	FromUI64(a.L, dest + 0);
	FromUI64(a.H, dest + 2);
}
uint UI128_y(UI128_t a)
{
	return UI64_y(a.L);
}

UI128_t UI128_Inv(UI128_t a)
{
	UI128_t ans;

	ans.L = UI64_Inv(a.L);
	ans.H = UI64_Inv(a.H);

	return ans;
}
UI128_t UI128_Add(UI128_t a, UI128_t b, UI128_t ans[2])
{
	UI64_t c[2];
	UI64_t d[2];
	UI64_t e[2];
	UI64_t f[2];
	UI128_t tmp[2];

	if (!ans)
		ans = tmp;

	UI64_Add(a.L, b.L, c);
	UI64_Add(a.H, b.H, d);
	UI64_Add(d[0], c[1], e);
	UI64_Add(d[1], e[1], f);

	ans[0].L = c[0];
	ans[0].H = e[0];
	ans[1].L = f[0];
	ans[1].H = UI64_0();

	return ans[0];
}
UI128_t UI128_Sub(UI128_t a, UI128_t b)
{
	UI128_t tmp[2];

	b = UI128_Inv(b);
	b = UI128_Add(b, UI128_x(1), tmp);

	return UI128_Add(a, b, tmp);
}
UI128_t UI128_Mul(UI128_t a, UI128_t b, UI128_t ans[2])
{
	UI64_t c[2];
	UI64_t d[2];
	UI64_t e[2];
	UI64_t f[2];
	UI64_t m[5][5];
	uint mc[5] = { 0 }; // max are 1, 3, 5, 5, 4
	uint i;
	UI128_t tmp[2];

	if (!ans)
		ans = tmp;

	UI64_Mul(a.L, b.L, c);
	UI64_Mul(a.L, b.H, d);
	UI64_Mul(a.H, b.L, e);
	UI64_Mul(a.H, b.H, f);

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
		UI64_t t = m[i][--mc[i]];
		UI64_t u = m[i][--mc[i]];
		UI64_t v[2];

		m[i][mc[i]++] = UI64_Add(t, u, v);
		m[i + 1][mc[i + 1]++] = v[1];
	}

	ans[0].L = m[0][0];
	ans[0].H = m[1][0];
	ans[1].L = m[2][0];
	ans[1].H = m[3][0];

	return ans[0];
}
static UI128_t DivSub(UI128_t a, UI128_t b, UI128_t ans[2], uint reti)
{
	UI128_t tmp[2];

	errorCase(UI128_IsZero(b));

	if (!ans)
		ans = tmp;

	ans[0] = UI128_0();

	if (UI64_IsZero(b.H))
	{
		{
			UI128_t dd;

			dd.L = UI64_0();
			dd.H = UI64_Div(a.H, b.L, NULL);

			ans[0] = UI128_Add(ans[0], dd, NULL);
			a = UI128_Sub(a, UI128_Mul(b, dd, NULL));
		}

		while (!UI64_IsZero(a.H))
		{
			UI128_t dd;
			UI64_t d2[2];

			UI64_Mul(UI64_Div(UI64_Fill(), b.L, NULL), a.H, d2);

			dd.L = d2[0];
			dd.H = d2[1];

			ans[0] = UI128_Add(ans[0], dd, NULL);
			a = UI128_Sub(a, UI128_Mul(b, dd, NULL));
		}

		{
			UI128_t dd;

			dd.L = UI64_Div(a.L, b.L, NULL);
			dd.H = UI64_0();

			ans[0] = UI128_Add(ans[0], dd, NULL);
			a = UI128_Sub(a, UI128_Mul(b, dd, NULL));
		}
	}
	else
	{
		if (!UI64_IsFill(b.H))
		{
			UI64_t c = UI64_Add(b.H, UI64_x(1), NULL);

			for (; ; )
			{
				UI64_t d = UI64_Div(a.H, c, NULL);
				UI128_t dd;

				if (UI64_IsZero(d))
					break;

				dd.L = d;
				dd.H = UI64_0();

				ans[0] = UI128_Add(ans[0], dd, NULL);
				a = UI128_Sub(a, UI128_Mul(b, dd, NULL));
			}
		}
		while (0 <= UI128_Comp(a, b))
		{
			ans[0] = UI128_Add(ans[0], UI128_x(1), NULL);
			a = UI128_Sub(a, b);
		}
	}
	ans[1] = a;
	return ans[reti];
}
UI128_t UI128_Div(UI128_t a, UI128_t b, UI128_t ans[2])
{
	return DivSub(a, b, ans, 0);
}
UI128_t UI128_Mod(UI128_t a, UI128_t b, UI128_t ans[2])
{
	return DivSub(a, b, ans, 1);
}

int UI128_IsZero(UI128_t a)
{
	return UI64_IsZero(a.L) && UI64_IsZero(a.H);
}
int UI128_IsFill(UI128_t a)
{
	return UI64_IsFill(a.L) && UI64_IsFill(a.H);
}
sint UI128_Comp(UI128_t a, UI128_t b)
{
	sint ret = UI64_Comp(a.H, b.H);

	if (!ret)
		ret = UI64_Comp(a.L, b.L);

	return ret;
}
UI128_t UI128_Fill(void)
{
	UI128_t ans;

	ans.L = UI64_Fill();
	ans.H = UI64_Fill();

	return ans;
}

UI128_t UI128_DivTwo_Rem(UI128_t a, uint *rem)
{
	UI128_t ans;

	ans.H = UI64_DivTwo_Rem(a.H, rem);
	ans.L = UI64_DivTwo_Rem(a.L, rem);

	return ans;
}
UI128_t UI128_DivTwo(UI128_t a)
{
	uint rem = 0;

	return UI128_DivTwo_Rem(a, &rem);
}
