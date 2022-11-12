#include "uint2048.h"

UI2048_t ToUI2048(uint src[64])
{
	UI2048_t ans;

	ans.L = ToUI1024(src + 0);
	ans.H = ToUI1024(src + 32);

	return ans;
}
UI2048_t UI2048_x(uint x)
{
	UI2048_t ans;

	ans.L = UI1024_x(x);
	ans.H = UI1024_0();

	return ans;
}
UI2048_t UI2048_0(void)
{
	UI2048_t ans;

	ans.L = UI1024_0();
	ans.H = UI1024_0();

	return ans;
}
void FromUI2048(UI2048_t a, uint dest[64])
{
	FromUI1024(a.L, dest + 0);
	FromUI1024(a.H, dest + 32);
}
uint UI2048_y(UI2048_t a)
{
	return UI1024_y(a.L);
}

UI2048_t UI2048_Inv(UI2048_t a)
{
	UI2048_t ans;

	ans.L = UI1024_Inv(a.L);
	ans.H = UI1024_Inv(a.H);

	return ans;
}
UI2048_t UI2048_Add(UI2048_t a, UI2048_t b, UI2048_t ans[2])
{
	UI1024_t c[2];
	UI1024_t d[2];
	UI1024_t e[2];
	UI1024_t f[2];
	UI2048_t tmp[2];

	if (!ans)
		ans = tmp;

	UI1024_Add(a.L, b.L, c);
	UI1024_Add(a.H, b.H, d);
	UI1024_Add(d[0], c[1], e);
	UI1024_Add(d[1], e[1], f);

	ans[0].L = c[0];
	ans[0].H = e[0];
	ans[1].L = f[0];
	ans[1].H = UI1024_0();

	return ans[0];
}
UI2048_t UI2048_Sub(UI2048_t a, UI2048_t b)
{
	UI2048_t tmp[2];

	b = UI2048_Inv(b);
	b = UI2048_Add(b, UI2048_x(1), tmp);

	return UI2048_Add(a, b, tmp);
}
UI2048_t UI2048_Mul(UI2048_t a, UI2048_t b, UI2048_t ans[2])
{
	UI1024_t c[2];
	UI1024_t d[2];
	UI1024_t e[2];
	UI1024_t f[2];
	UI1024_t m[5][5];
	uint mc[5] = { 0 }; // max are 1, 3, 5, 5, 4
	uint i;
	UI2048_t tmp[2];

	if (!ans)
		ans = tmp;

	UI1024_Mul(a.L, b.L, c);
	UI1024_Mul(a.L, b.H, d);
	UI1024_Mul(a.H, b.L, e);
	UI1024_Mul(a.H, b.H, f);

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
		UI1024_t t = m[i][--mc[i]];
		UI1024_t u = m[i][--mc[i]];
		UI1024_t v[2];

		m[i][mc[i]++] = UI1024_Add(t, u, v);
		m[i + 1][mc[i + 1]++] = v[1];
	}

	ans[0].L = m[0][0];
	ans[0].H = m[1][0];
	ans[1].L = m[2][0];
	ans[1].H = m[3][0];

	return ans[0];
}
static UI2048_t DivSub(UI2048_t a, UI2048_t b, UI2048_t ans[2], uint reti)
{
	UI2048_t tmp[2];

	errorCase(UI2048_IsZero(b));

	if (!ans)
		ans = tmp;

	ans[0] = UI2048_0();

	if (UI1024_IsZero(b.H))
	{
		{
			UI2048_t dd;

			dd.L = UI1024_0();
			dd.H = UI1024_Div(a.H, b.L, NULL);

			ans[0] = UI2048_Add(ans[0], dd, NULL);
			a = UI2048_Sub(a, UI2048_Mul(b, dd, NULL));
		}

		while (!UI1024_IsZero(a.H))
		{
			UI2048_t dd;
			UI1024_t d2[2];

			UI1024_Mul(UI1024_Div(UI1024_Fill(), b.L, NULL), a.H, d2);

			dd.L = d2[0];
			dd.H = d2[1];

			ans[0] = UI2048_Add(ans[0], dd, NULL);
			a = UI2048_Sub(a, UI2048_Mul(b, dd, NULL));
		}

		{
			UI2048_t dd;

			dd.L = UI1024_Div(a.L, b.L, NULL);
			dd.H = UI1024_0();

			ans[0] = UI2048_Add(ans[0], dd, NULL);
			a = UI2048_Sub(a, UI2048_Mul(b, dd, NULL));
		}
	}
	else
	{
		if (!UI1024_IsFill(b.H))
		{
			UI1024_t c = UI1024_Add(b.H, UI1024_x(1), NULL);

			for (; ; )
			{
				UI1024_t d = UI1024_Div(a.H, c, NULL);
				UI2048_t dd;

				if (UI1024_IsZero(d))
					break;

				dd.L = d;
				dd.H = UI1024_0();

				ans[0] = UI2048_Add(ans[0], dd, NULL);
				a = UI2048_Sub(a, UI2048_Mul(b, dd, NULL));
			}
		}
		while (0 <= UI2048_Comp(a, b))
		{
			ans[0] = UI2048_Add(ans[0], UI2048_x(1), NULL);
			a = UI2048_Sub(a, b);
		}
	}
	ans[1] = a;
	return ans[reti];
}
UI2048_t UI2048_Div(UI2048_t a, UI2048_t b, UI2048_t ans[2])
{
	return DivSub(a, b, ans, 0);
}
UI2048_t UI2048_Mod(UI2048_t a, UI2048_t b, UI2048_t ans[2])
{
	return DivSub(a, b, ans, 1);
}

int UI2048_IsZero(UI2048_t a)
{
	return UI1024_IsZero(a.L) && UI1024_IsZero(a.H);
}
int UI2048_IsFill(UI2048_t a)
{
	return UI1024_IsFill(a.L) && UI1024_IsFill(a.H);
}
sint UI2048_Comp(UI2048_t a, UI2048_t b)
{
	sint ret = UI1024_Comp(a.H, b.H);

	if (!ret)
		ret = UI1024_Comp(a.L, b.L);

	return ret;
}
UI2048_t UI2048_Fill(void)
{
	UI2048_t ans;

	ans.L = UI1024_Fill();
	ans.H = UI1024_Fill();

	return ans;
}

UI2048_t UI2048_DivTwo_Rem(UI2048_t a, uint *rem)
{
	UI2048_t ans;

	ans.H = UI1024_DivTwo_Rem(a.H, rem);
	ans.L = UI1024_DivTwo_Rem(a.L, rem);

	return ans;
}
UI2048_t UI2048_DivTwo(UI2048_t a)
{
	uint rem = 0;

	return UI2048_DivTwo_Rem(a, &rem);
}
