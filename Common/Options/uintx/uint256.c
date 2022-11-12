#include "uint256.h"

UI256_t ToUI256(uint src[8])
{
	UI256_t ans;

	ans.L = ToUI128(src + 0);
	ans.H = ToUI128(src + 4);

	return ans;
}
UI256_t UI256_x(uint x)
{
	UI256_t ans;

	ans.L = UI128_x(x);
	ans.H = UI128_0();

	return ans;
}
UI256_t UI256_0(void)
{
	UI256_t ans;

	ans.L = UI128_0();
	ans.H = UI128_0();

	return ans;
}
void FromUI256(UI256_t a, uint dest[8])
{
	FromUI128(a.L, dest + 0);
	FromUI128(a.H, dest + 4);
}
uint UI256_y(UI256_t a)
{
	return UI128_y(a.L);
}

UI256_t UI256_Inv(UI256_t a)
{
	UI256_t ans;

	ans.L = UI128_Inv(a.L);
	ans.H = UI128_Inv(a.H);

	return ans;
}
UI256_t UI256_Add(UI256_t a, UI256_t b, UI256_t ans[2])
{
	UI128_t c[2];
	UI128_t d[2];
	UI128_t e[2];
	UI128_t f[2];
	UI256_t tmp[2];

	if (!ans)
		ans = tmp;

	UI128_Add(a.L, b.L, c);
	UI128_Add(a.H, b.H, d);
	UI128_Add(d[0], c[1], e);
	UI128_Add(d[1], e[1], f);

	ans[0].L = c[0];
	ans[0].H = e[0];
	ans[1].L = f[0];
	ans[1].H = UI128_0();

	return ans[0];
}
UI256_t UI256_Sub(UI256_t a, UI256_t b)
{
	UI256_t tmp[2];

	b = UI256_Inv(b);
	b = UI256_Add(b, UI256_x(1), tmp);

	return UI256_Add(a, b, tmp);
}
UI256_t UI256_Mul(UI256_t a, UI256_t b, UI256_t ans[2])
{
	UI128_t c[2];
	UI128_t d[2];
	UI128_t e[2];
	UI128_t f[2];
	UI128_t m[5][5];
	uint mc[5] = { 0 }; // max are 1, 3, 5, 5, 4
	uint i;
	UI256_t tmp[2];

	if (!ans)
		ans = tmp;

	UI128_Mul(a.L, b.L, c);
	UI128_Mul(a.L, b.H, d);
	UI128_Mul(a.H, b.L, e);
	UI128_Mul(a.H, b.H, f);

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
		UI128_t t = m[i][--mc[i]];
		UI128_t u = m[i][--mc[i]];
		UI128_t v[2];

		m[i][mc[i]++] = UI128_Add(t, u, v);
		m[i + 1][mc[i + 1]++] = v[1];
	}

	ans[0].L = m[0][0];
	ans[0].H = m[1][0];
	ans[1].L = m[2][0];
	ans[1].H = m[3][0];

	return ans[0];
}
static UI256_t DivSub(UI256_t a, UI256_t b, UI256_t ans[2], uint reti)
{
	UI256_t tmp[2];

	errorCase(UI256_IsZero(b));

	if (!ans)
		ans = tmp;

	ans[0] = UI256_0();

	if (UI128_IsZero(b.H))
	{
		{
			UI256_t dd;

			dd.L = UI128_0();
			dd.H = UI128_Div(a.H, b.L, NULL);

			ans[0] = UI256_Add(ans[0], dd, NULL);
			a = UI256_Sub(a, UI256_Mul(b, dd, NULL));
		}

		while (!UI128_IsZero(a.H))
		{
			UI256_t dd;
			UI128_t d2[2];

			UI128_Mul(UI128_Div(UI128_Fill(), b.L, NULL), a.H, d2);

			dd.L = d2[0];
			dd.H = d2[1];

			ans[0] = UI256_Add(ans[0], dd, NULL);
			a = UI256_Sub(a, UI256_Mul(b, dd, NULL));
		}

		{
			UI256_t dd;

			dd.L = UI128_Div(a.L, b.L, NULL);
			dd.H = UI128_0();

			ans[0] = UI256_Add(ans[0], dd, NULL);
			a = UI256_Sub(a, UI256_Mul(b, dd, NULL));
		}
	}
	else
	{
		if (!UI128_IsFill(b.H))
		{
			UI128_t c = UI128_Add(b.H, UI128_x(1), NULL);

			for (; ; )
			{
				UI128_t d = UI128_Div(a.H, c, NULL);
				UI256_t dd;

				if (UI128_IsZero(d))
					break;

				dd.L = d;
				dd.H = UI128_0();

				ans[0] = UI256_Add(ans[0], dd, NULL);
				a = UI256_Sub(a, UI256_Mul(b, dd, NULL));
			}
		}
		while (0 <= UI256_Comp(a, b))
		{
			ans[0] = UI256_Add(ans[0], UI256_x(1), NULL);
			a = UI256_Sub(a, b);
		}
	}
	ans[1] = a;
	return ans[reti];
}
UI256_t UI256_Div(UI256_t a, UI256_t b, UI256_t ans[2])
{
	return DivSub(a, b, ans, 0);
}
UI256_t UI256_Mod(UI256_t a, UI256_t b, UI256_t ans[2])
{
	return DivSub(a, b, ans, 1);
}

int UI256_IsZero(UI256_t a)
{
	return UI128_IsZero(a.L) && UI128_IsZero(a.H);
}
int UI256_IsFill(UI256_t a)
{
	return UI128_IsFill(a.L) && UI128_IsFill(a.H);
}
sint UI256_Comp(UI256_t a, UI256_t b)
{
	sint ret = UI128_Comp(a.H, b.H);

	if (!ret)
		ret = UI128_Comp(a.L, b.L);

	return ret;
}
UI256_t UI256_Fill(void)
{
	UI256_t ans;

	ans.L = UI128_Fill();
	ans.H = UI128_Fill();

	return ans;
}

UI256_t UI256_DivTwo_Rem(UI256_t a, uint *rem)
{
	UI256_t ans;

	ans.H = UI128_DivTwo_Rem(a.H, rem);
	ans.L = UI128_DivTwo_Rem(a.L, rem);

	return ans;
}
UI256_t UI256_DivTwo(UI256_t a)
{
	uint rem = 0;

	return UI256_DivTwo_Rem(a, &rem);
}
