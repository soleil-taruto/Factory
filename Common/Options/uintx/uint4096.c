#include "uint4096.h"

UI4096_t ToUI4096(uint src[128])
{
	UI4096_t ans;

	ans.L = ToUI2048(src + 0);
	ans.H = ToUI2048(src + 64);

	return ans;
}
UI4096_t UI4096_x(uint x)
{
	UI4096_t ans;

	ans.L = UI2048_x(x);
	ans.H = UI2048_0();

	return ans;
}
UI4096_t UI4096_0(void)
{
	UI4096_t ans;

	ans.L = UI2048_0();
	ans.H = UI2048_0();

	return ans;
}
void FromUI4096(UI4096_t a, uint dest[128])
{
	FromUI2048(a.L, dest + 0);
	FromUI2048(a.H, dest + 64);
}
uint UI4096_y(UI4096_t a)
{
	return UI2048_y(a.L);
}

UI4096_t UI4096_Inv(UI4096_t a)
{
	UI4096_t ans;

	ans.L = UI2048_Inv(a.L);
	ans.H = UI2048_Inv(a.H);

	return ans;
}
UI4096_t UI4096_Add(UI4096_t a, UI4096_t b, UI4096_t ans[2])
{
	UI2048_t c[2];
	UI2048_t d[2];
	UI2048_t e[2];
	UI2048_t f[2];
	UI4096_t tmp[2];

	if (!ans)
		ans = tmp;

	UI2048_Add(a.L, b.L, c);
	UI2048_Add(a.H, b.H, d);
	UI2048_Add(d[0], c[1], e);
	UI2048_Add(d[1], e[1], f);

	ans[0].L = c[0];
	ans[0].H = e[0];
	ans[1].L = f[0];
	ans[1].H = UI2048_0();

	return ans[0];
}
UI4096_t UI4096_Sub(UI4096_t a, UI4096_t b)
{
	UI4096_t tmp[2];

	b = UI4096_Inv(b);
	b = UI4096_Add(b, UI4096_x(1), tmp);

	return UI4096_Add(a, b, tmp);
}
UI4096_t UI4096_Mul(UI4096_t a, UI4096_t b, UI4096_t ans[2])
{
	UI2048_t c[2];
	UI2048_t d[2];
	UI2048_t e[2];
	UI2048_t f[2];
	UI2048_t m[5][5];
	uint mc[5] = { 0 }; // max are 1, 3, 5, 5, 4
	uint i;
	UI4096_t tmp[2];

	if (!ans)
		ans = tmp;

	UI2048_Mul(a.L, b.L, c);
	UI2048_Mul(a.L, b.H, d);
	UI2048_Mul(a.H, b.L, e);
	UI2048_Mul(a.H, b.H, f);

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
		UI2048_t t = m[i][--mc[i]];
		UI2048_t u = m[i][--mc[i]];
		UI2048_t v[2];

		m[i][mc[i]++] = UI2048_Add(t, u, v);
		m[i + 1][mc[i + 1]++] = v[1];
	}

	ans[0].L = m[0][0];
	ans[0].H = m[1][0];
	ans[1].L = m[2][0];
	ans[1].H = m[3][0];

	return ans[0];
}
static UI4096_t DivSub(UI4096_t a, UI4096_t b, UI4096_t ans[2], uint reti)
{
	UI4096_t tmp[2];

	errorCase(UI4096_IsZero(b));

	if (!ans)
		ans = tmp;

	ans[0] = UI4096_0();

	if (UI2048_IsZero(b.H))
	{
		{
			UI4096_t dd;

			dd.L = UI2048_0();
			dd.H = UI2048_Div(a.H, b.L, NULL);

			ans[0] = UI4096_Add(ans[0], dd, NULL);
			a = UI4096_Sub(a, UI4096_Mul(b, dd, NULL));
		}

		while (!UI2048_IsZero(a.H))
		{
			UI4096_t dd;
			UI2048_t d2[2];

			UI2048_Mul(UI2048_Div(UI2048_Fill(), b.L, NULL), a.H, d2);

			dd.L = d2[0];
			dd.H = d2[1];

			ans[0] = UI4096_Add(ans[0], dd, NULL);
			a = UI4096_Sub(a, UI4096_Mul(b, dd, NULL));
		}

		{
			UI4096_t dd;

			dd.L = UI2048_Div(a.L, b.L, NULL);
			dd.H = UI2048_0();

			ans[0] = UI4096_Add(ans[0], dd, NULL);
			a = UI4096_Sub(a, UI4096_Mul(b, dd, NULL));
		}
	}
	else
	{
		if (!UI2048_IsFill(b.H))
		{
			UI2048_t c = UI2048_Add(b.H, UI2048_x(1), NULL);

			for (; ; )
			{
				UI2048_t d = UI2048_Div(a.H, c, NULL);
				UI4096_t dd;

				if (UI2048_IsZero(d))
					break;

				dd.L = d;
				dd.H = UI2048_0();

				ans[0] = UI4096_Add(ans[0], dd, NULL);
				a = UI4096_Sub(a, UI4096_Mul(b, dd, NULL));
			}
		}
		while (0 <= UI4096_Comp(a, b))
		{
			ans[0] = UI4096_Add(ans[0], UI4096_x(1), NULL);
			a = UI4096_Sub(a, b);
		}
	}
	ans[1] = a;
	return ans[reti];
}
UI4096_t UI4096_Div(UI4096_t a, UI4096_t b, UI4096_t ans[2])
{
	return DivSub(a, b, ans, 0);
}
UI4096_t UI4096_Mod(UI4096_t a, UI4096_t b, UI4096_t ans[2])
{
	return DivSub(a, b, ans, 1);
}

int UI4096_IsZero(UI4096_t a)
{
	return UI2048_IsZero(a.L) && UI2048_IsZero(a.H);
}
int UI4096_IsFill(UI4096_t a)
{
	return UI2048_IsFill(a.L) && UI2048_IsFill(a.H);
}
sint UI4096_Comp(UI4096_t a, UI4096_t b)
{
	sint ret = UI2048_Comp(a.H, b.H);

	if (!ret)
		ret = UI2048_Comp(a.L, b.L);

	return ret;
}
UI4096_t UI4096_Fill(void)
{
	UI4096_t ans;

	ans.L = UI2048_Fill();
	ans.H = UI2048_Fill();

	return ans;
}

UI4096_t UI4096_DivTwo_Rem(UI4096_t a, uint *rem)
{
	UI4096_t ans;

	ans.H = UI2048_DivTwo_Rem(a.H, rem);
	ans.L = UI2048_DivTwo_Rem(a.L, rem);

	return ans;
}
UI4096_t UI4096_DivTwo(UI4096_t a)
{
	uint rem = 0;

	return UI4096_DivTwo_Rem(a, &rem);
}
