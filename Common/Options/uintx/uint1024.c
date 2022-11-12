#include "uint1024.h"

UI1024_t ToUI1024(uint src[32])
{
	UI1024_t ans;

	ans.L = ToUI512(src + 0);
	ans.H = ToUI512(src + 16);

	return ans;
}
UI1024_t UI1024_x(uint x)
{
	UI1024_t ans;

	ans.L = UI512_x(x);
	ans.H = UI512_0();

	return ans;
}
UI1024_t UI1024_0(void)
{
	UI1024_t ans;

	ans.L = UI512_0();
	ans.H = UI512_0();

	return ans;
}
void FromUI1024(UI1024_t a, uint dest[32])
{
	FromUI512(a.L, dest + 0);
	FromUI512(a.H, dest + 16);
}
uint UI1024_y(UI1024_t a)
{
	return UI512_y(a.L);
}

UI1024_t UI1024_Inv(UI1024_t a)
{
	UI1024_t ans;

	ans.L = UI512_Inv(a.L);
	ans.H = UI512_Inv(a.H);

	return ans;
}
UI1024_t UI1024_Add(UI1024_t a, UI1024_t b, UI1024_t ans[2])
{
	UI512_t c[2];
	UI512_t d[2];
	UI512_t e[2];
	UI512_t f[2];
	UI1024_t tmp[2];

	if (!ans)
		ans = tmp;

	UI512_Add(a.L, b.L, c);
	UI512_Add(a.H, b.H, d);
	UI512_Add(d[0], c[1], e);
	UI512_Add(d[1], e[1], f);

	ans[0].L = c[0];
	ans[0].H = e[0];
	ans[1].L = f[0];
	ans[1].H = UI512_0();

	return ans[0];
}
UI1024_t UI1024_Sub(UI1024_t a, UI1024_t b)
{
	UI1024_t tmp[2];

	b = UI1024_Inv(b);
	b = UI1024_Add(b, UI1024_x(1), tmp);

	return UI1024_Add(a, b, tmp);
}
UI1024_t UI1024_Mul(UI1024_t a, UI1024_t b, UI1024_t ans[2])
{
	UI512_t c[2];
	UI512_t d[2];
	UI512_t e[2];
	UI512_t f[2];
	UI512_t m[5][5];
	uint mc[5] = { 0 }; // max are 1, 3, 5, 5, 4
	uint i;
	UI1024_t tmp[2];

	if (!ans)
		ans = tmp;

	UI512_Mul(a.L, b.L, c);
	UI512_Mul(a.L, b.H, d);
	UI512_Mul(a.H, b.L, e);
	UI512_Mul(a.H, b.H, f);

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
		UI512_t t = m[i][--mc[i]];
		UI512_t u = m[i][--mc[i]];
		UI512_t v[2];

		m[i][mc[i]++] = UI512_Add(t, u, v);
		m[i + 1][mc[i + 1]++] = v[1];
	}

	ans[0].L = m[0][0];
	ans[0].H = m[1][0];
	ans[1].L = m[2][0];
	ans[1].H = m[3][0];

	return ans[0];
}
static UI1024_t DivSub(UI1024_t a, UI1024_t b, UI1024_t ans[2], uint reti)
{
	UI1024_t tmp[2];

	errorCase(UI1024_IsZero(b));

	if (!ans)
		ans = tmp;

	ans[0] = UI1024_0();

	if (UI512_IsZero(b.H))
	{
		{
			UI1024_t dd;

			dd.L = UI512_0();
			dd.H = UI512_Div(a.H, b.L, NULL);

			ans[0] = UI1024_Add(ans[0], dd, NULL);
			a = UI1024_Sub(a, UI1024_Mul(b, dd, NULL));
		}

		while (!UI512_IsZero(a.H))
		{
			UI1024_t dd;
			UI512_t d2[2];

			UI512_Mul(UI512_Div(UI512_Fill(), b.L, NULL), a.H, d2);

			dd.L = d2[0];
			dd.H = d2[1];

			ans[0] = UI1024_Add(ans[0], dd, NULL);
			a = UI1024_Sub(a, UI1024_Mul(b, dd, NULL));
		}

		{
			UI1024_t dd;

			dd.L = UI512_Div(a.L, b.L, NULL);
			dd.H = UI512_0();

			ans[0] = UI1024_Add(ans[0], dd, NULL);
			a = UI1024_Sub(a, UI1024_Mul(b, dd, NULL));
		}
	}
	else
	{
		if (!UI512_IsFill(b.H))
		{
			UI512_t c = UI512_Add(b.H, UI512_x(1), NULL);

			for (; ; )
			{
				UI512_t d = UI512_Div(a.H, c, NULL);
				UI1024_t dd;

				if (UI512_IsZero(d))
					break;

				dd.L = d;
				dd.H = UI512_0();

				ans[0] = UI1024_Add(ans[0], dd, NULL);
				a = UI1024_Sub(a, UI1024_Mul(b, dd, NULL));
			}
		}
		while (0 <= UI1024_Comp(a, b))
		{
			ans[0] = UI1024_Add(ans[0], UI1024_x(1), NULL);
			a = UI1024_Sub(a, b);
		}
	}
	ans[1] = a;
	return ans[reti];
}
UI1024_t UI1024_Div(UI1024_t a, UI1024_t b, UI1024_t ans[2])
{
	return DivSub(a, b, ans, 0);
}
UI1024_t UI1024_Mod(UI1024_t a, UI1024_t b, UI1024_t ans[2])
{
	return DivSub(a, b, ans, 1);
}

int UI1024_IsZero(UI1024_t a)
{
	return UI512_IsZero(a.L) && UI512_IsZero(a.H);
}
int UI1024_IsFill(UI1024_t a)
{
	return UI512_IsFill(a.L) && UI512_IsFill(a.H);
}
sint UI1024_Comp(UI1024_t a, UI1024_t b)
{
	sint ret = UI512_Comp(a.H, b.H);

	if (!ret)
		ret = UI512_Comp(a.L, b.L);

	return ret;
}
UI1024_t UI1024_Fill(void)
{
	UI1024_t ans;

	ans.L = UI512_Fill();
	ans.H = UI512_Fill();

	return ans;
}

UI1024_t UI1024_DivTwo_Rem(UI1024_t a, uint *rem)
{
	UI1024_t ans;

	ans.H = UI512_DivTwo_Rem(a.H, rem);
	ans.L = UI512_DivTwo_Rem(a.L, rem);

	return ans;
}
UI1024_t UI1024_DivTwo(UI1024_t a)
{
	uint rem = 0;

	return UI1024_DivTwo_Rem(a, &rem);
}
