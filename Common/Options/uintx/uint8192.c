#include "uint8192.h"

UI8192_t ToUI8192(uint src[256])
{
	UI8192_t ans;

	ans.L = ToUI4096(src + 0);
	ans.H = ToUI4096(src + 128);

	return ans;
}
UI8192_t UI8192_x(uint x)
{
	UI8192_t ans;

	ans.L = UI4096_x(x);
	ans.H = UI4096_0();

	return ans;
}
UI8192_t UI8192_0(void)
{
	UI8192_t ans;

	ans.L = UI4096_0();
	ans.H = UI4096_0();

	return ans;
}
void FromUI8192(UI8192_t a, uint dest[256])
{
	FromUI4096(a.L, dest + 0);
	FromUI4096(a.H, dest + 128);
}
uint UI8192_y(UI8192_t a)
{
	return UI4096_y(a.L);
}

UI8192_t UI8192_Inv(UI8192_t a)
{
	UI8192_t ans;

	ans.L = UI4096_Inv(a.L);
	ans.H = UI4096_Inv(a.H);

	return ans;
}
UI8192_t UI8192_Add(UI8192_t a, UI8192_t b, UI8192_t ans[2])
{
	UI4096_t c[2];
	UI4096_t d[2];
	UI4096_t e[2];
	UI4096_t f[2];
	UI8192_t tmp[2];

	if (!ans)
		ans = tmp;

	UI4096_Add(a.L, b.L, c);
	UI4096_Add(a.H, b.H, d);
	UI4096_Add(d[0], c[1], e);
	UI4096_Add(d[1], e[1], f);

	ans[0].L = c[0];
	ans[0].H = e[0];
	ans[1].L = f[0];
	ans[1].H = UI4096_0();

	return ans[0];
}
UI8192_t UI8192_Sub(UI8192_t a, UI8192_t b)
{
	UI8192_t tmp[2];

	b = UI8192_Inv(b);
	b = UI8192_Add(b, UI8192_x(1), tmp);

	return UI8192_Add(a, b, tmp);
}
UI8192_t UI8192_Mul(UI8192_t a, UI8192_t b, UI8192_t ans[2])
{
	UI4096_t c[2];
	UI4096_t d[2];
	UI4096_t e[2];
	UI4096_t f[2];
	UI4096_t m[5][5];
	uint mc[5] = { 0 }; // max are 1, 3, 5, 5, 4
	uint i;
	UI8192_t tmp[2];

	if (!ans)
		ans = tmp;

	UI4096_Mul(a.L, b.L, c);
	UI4096_Mul(a.L, b.H, d);
	UI4096_Mul(a.H, b.L, e);
	UI4096_Mul(a.H, b.H, f);

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
		UI4096_t t = m[i][--mc[i]];
		UI4096_t u = m[i][--mc[i]];
		UI4096_t v[2];

		m[i][mc[i]++] = UI4096_Add(t, u, v);
		m[i + 1][mc[i + 1]++] = v[1];
	}

	ans[0].L = m[0][0];
	ans[0].H = m[1][0];
	ans[1].L = m[2][0];
	ans[1].H = m[3][0];

	return ans[0];
}
static UI8192_t DivSub(UI8192_t a, UI8192_t b, UI8192_t ans[2], uint reti)
{
	UI8192_t tmp[2];

	errorCase(UI8192_IsZero(b));

	if (!ans)
		ans = tmp;

	ans[0] = UI8192_0();

	if (UI4096_IsZero(b.H))
	{
		{
			UI8192_t dd;

			dd.L = UI4096_0();
			dd.H = UI4096_Div(a.H, b.L, NULL);

			ans[0] = UI8192_Add(ans[0], dd, NULL);
			a = UI8192_Sub(a, UI8192_Mul(b, dd, NULL));
		}

		while (!UI4096_IsZero(a.H))
		{
			UI8192_t dd;
			UI4096_t d2[2];

			UI4096_Mul(UI4096_Div(UI4096_Fill(), b.L, NULL), a.H, d2);

			dd.L = d2[0];
			dd.H = d2[1];

			ans[0] = UI8192_Add(ans[0], dd, NULL);
			a = UI8192_Sub(a, UI8192_Mul(b, dd, NULL));
		}

		{
			UI8192_t dd;

			dd.L = UI4096_Div(a.L, b.L, NULL);
			dd.H = UI4096_0();

			ans[0] = UI8192_Add(ans[0], dd, NULL);
			a = UI8192_Sub(a, UI8192_Mul(b, dd, NULL));
		}
	}
	else
	{
		if (!UI4096_IsFill(b.H))
		{
			UI4096_t c = UI4096_Add(b.H, UI4096_x(1), NULL);

			for (; ; )
			{
				UI4096_t d = UI4096_Div(a.H, c, NULL);
				UI8192_t dd;

				if (UI4096_IsZero(d))
					break;

				dd.L = d;
				dd.H = UI4096_0();

				ans[0] = UI8192_Add(ans[0], dd, NULL);
				a = UI8192_Sub(a, UI8192_Mul(b, dd, NULL));
			}
		}
		while (0 <= UI8192_Comp(a, b))
		{
			ans[0] = UI8192_Add(ans[0], UI8192_x(1), NULL);
			a = UI8192_Sub(a, b);
		}
	}
	ans[1] = a;
	return ans[reti];
}
UI8192_t UI8192_Div(UI8192_t a, UI8192_t b, UI8192_t ans[2])
{
	return DivSub(a, b, ans, 0);
}
UI8192_t UI8192_Mod(UI8192_t a, UI8192_t b, UI8192_t ans[2])
{
	return DivSub(a, b, ans, 1);
}

int UI8192_IsZero(UI8192_t a)
{
	return UI4096_IsZero(a.L) && UI4096_IsZero(a.H);
}
int UI8192_IsFill(UI8192_t a)
{
	return UI4096_IsFill(a.L) && UI4096_IsFill(a.H);
}
sint UI8192_Comp(UI8192_t a, UI8192_t b)
{
	sint ret = UI4096_Comp(a.H, b.H);

	if (!ret)
		ret = UI4096_Comp(a.L, b.L);

	return ret;
}
UI8192_t UI8192_Fill(void)
{
	UI8192_t ans;

	ans.L = UI4096_Fill();
	ans.H = UI4096_Fill();

	return ans;
}

UI8192_t UI8192_DivTwo_Rem(UI8192_t a, uint *rem)
{
	UI8192_t ans;

	ans.H = UI4096_DivTwo_Rem(a.H, rem);
	ans.L = UI4096_DivTwo_Rem(a.L, rem);

	return ans;
}
UI8192_t UI8192_DivTwo(UI8192_t a)
{
	uint rem = 0;

	return UI8192_DivTwo_Rem(a, &rem);
}
