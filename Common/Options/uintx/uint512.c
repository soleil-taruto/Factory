#include "uint512.h"

UI512_t ToUI512(uint src[16])
{
	UI512_t ans;

	ans.L = ToUI256(src + 0);
	ans.H = ToUI256(src + 8);

	return ans;
}
UI512_t UI512_x(uint x)
{
	UI512_t ans;

	ans.L = UI256_x(x);
	ans.H = UI256_0();

	return ans;
}
UI512_t UI512_0(void)
{
	UI512_t ans;

	ans.L = UI256_0();
	ans.H = UI256_0();

	return ans;
}
void FromUI512(UI512_t a, uint dest[16])
{
	FromUI256(a.L, dest + 0);
	FromUI256(a.H, dest + 8);
}
uint UI512_y(UI512_t a)
{
	return UI256_y(a.L);
}

UI512_t UI512_Inv(UI512_t a)
{
	UI512_t ans;

	ans.L = UI256_Inv(a.L);
	ans.H = UI256_Inv(a.H);

	return ans;
}
UI512_t UI512_Add(UI512_t a, UI512_t b, UI512_t ans[2])
{
	UI256_t c[2];
	UI256_t d[2];
	UI256_t e[2];
	UI256_t f[2];
	UI512_t tmp[2];

	if (!ans)
		ans = tmp;

	UI256_Add(a.L, b.L, c);
	UI256_Add(a.H, b.H, d);
	UI256_Add(d[0], c[1], e);
	UI256_Add(d[1], e[1], f);

	ans[0].L = c[0];
	ans[0].H = e[0];
	ans[1].L = f[0];
	ans[1].H = UI256_0();

	return ans[0];
}
UI512_t UI512_Sub(UI512_t a, UI512_t b)
{
	UI512_t tmp[2];

	b = UI512_Inv(b);
	b = UI512_Add(b, UI512_x(1), tmp);

	return UI512_Add(a, b, tmp);
}
UI512_t UI512_Mul(UI512_t a, UI512_t b, UI512_t ans[2])
{
	UI256_t c[2];
	UI256_t d[2];
	UI256_t e[2];
	UI256_t f[2];
	UI256_t m[5][5];
	uint mc[5] = { 0 }; // max are 1, 3, 5, 5, 4
	uint i;
	UI512_t tmp[2];

	if (!ans)
		ans = tmp;

	UI256_Mul(a.L, b.L, c);
	UI256_Mul(a.L, b.H, d);
	UI256_Mul(a.H, b.L, e);
	UI256_Mul(a.H, b.H, f);

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
		UI256_t t = m[i][--mc[i]];
		UI256_t u = m[i][--mc[i]];
		UI256_t v[2];

		m[i][mc[i]++] = UI256_Add(t, u, v);
		m[i + 1][mc[i + 1]++] = v[1];
	}

	ans[0].L = m[0][0];
	ans[0].H = m[1][0];
	ans[1].L = m[2][0];
	ans[1].H = m[3][0];

	return ans[0];
}
static UI512_t DivSub(UI512_t a, UI512_t b, UI512_t ans[2], uint reti)
{
	UI512_t tmp[2];

	errorCase(UI512_IsZero(b));

	if (!ans)
		ans = tmp;

	ans[0] = UI512_0();

	if (UI256_IsZero(b.H))
	{
		{
			UI512_t dd;

			dd.L = UI256_0();
			dd.H = UI256_Div(a.H, b.L, NULL);

			ans[0] = UI512_Add(ans[0], dd, NULL);
			a = UI512_Sub(a, UI512_Mul(b, dd, NULL));
		}

		while (!UI256_IsZero(a.H))
		{
			UI512_t dd;
			UI256_t d2[2];

			UI256_Mul(UI256_Div(UI256_Fill(), b.L, NULL), a.H, d2);

			dd.L = d2[0];
			dd.H = d2[1];

			ans[0] = UI512_Add(ans[0], dd, NULL);
			a = UI512_Sub(a, UI512_Mul(b, dd, NULL));
		}

		{
			UI512_t dd;

			dd.L = UI256_Div(a.L, b.L, NULL);
			dd.H = UI256_0();

			ans[0] = UI512_Add(ans[0], dd, NULL);
			a = UI512_Sub(a, UI512_Mul(b, dd, NULL));
		}
	}
	else
	{
		if (!UI256_IsFill(b.H))
		{
			UI256_t c = UI256_Add(b.H, UI256_x(1), NULL);

			for (; ; )
			{
				UI256_t d = UI256_Div(a.H, c, NULL);
				UI512_t dd;

				if (UI256_IsZero(d))
					break;

				dd.L = d;
				dd.H = UI256_0();

				ans[0] = UI512_Add(ans[0], dd, NULL);
				a = UI512_Sub(a, UI512_Mul(b, dd, NULL));
			}
		}
		while (0 <= UI512_Comp(a, b))
		{
			ans[0] = UI512_Add(ans[0], UI512_x(1), NULL);
			a = UI512_Sub(a, b);
		}
	}
	ans[1] = a;
	return ans[reti];
}
UI512_t UI512_Div(UI512_t a, UI512_t b, UI512_t ans[2])
{
	return DivSub(a, b, ans, 0);
}
UI512_t UI512_Mod(UI512_t a, UI512_t b, UI512_t ans[2])
{
	return DivSub(a, b, ans, 1);
}

int UI512_IsZero(UI512_t a)
{
	return UI256_IsZero(a.L) && UI256_IsZero(a.H);
}
int UI512_IsFill(UI512_t a)
{
	return UI256_IsFill(a.L) && UI256_IsFill(a.H);
}
sint UI512_Comp(UI512_t a, UI512_t b)
{
	sint ret = UI256_Comp(a.H, b.H);

	if (!ret)
		ret = UI256_Comp(a.L, b.L);

	return ret;
}
UI512_t UI512_Fill(void)
{
	UI512_t ans;

	ans.L = UI256_Fill();
	ans.H = UI256_Fill();

	return ans;
}

UI512_t UI512_DivTwo_Rem(UI512_t a, uint *rem)
{
	UI512_t ans;

	ans.H = UI256_DivTwo_Rem(a.H, rem);
	ans.L = UI256_DivTwo_Rem(a.L, rem);

	return ans;
}
UI512_t UI512_DivTwo(UI512_t a)
{
	uint rem = 0;

	return UI512_DivTwo_Rem(a, &rem);
}
