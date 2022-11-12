#include "uint$BIT.h"

UI$BIT_t ToUI$BIT(uint src[$SZ])
{
	UI$BIT_t ans;

	ans.L = ToUI$hBIT(src + 0);
	ans.H = ToUI$hBIT(src + $hSZ);

	return ans;
}
UI$BIT_t UI$BIT_x(uint x)
{
	UI$BIT_t ans;

	ans.L = UI$hBIT_x(x);
	ans.H = UI$hBIT_0();

	return ans;
}
UI$BIT_t UI$BIT_0(void)
{
	UI$BIT_t ans;

	ans.L = UI$hBIT_0();
	ans.H = UI$hBIT_0();

	return ans;
}
void FromUI$BIT(UI$BIT_t a, uint dest[$SZ])
{
	FromUI$hBIT(a.L, dest + 0);
	FromUI$hBIT(a.H, dest + $hSZ);
}
uint UI$BIT_y(UI$BIT_t a)
{
	return UI$hBIT_y(a.L);
}

UI$BIT_t UI$BIT_Inv(UI$BIT_t a)
{
	UI$BIT_t ans;

	ans.L = UI$hBIT_Inv(a.L);
	ans.H = UI$hBIT_Inv(a.H);

	return ans;
}
UI$BIT_t UI$BIT_Add(UI$BIT_t a, UI$BIT_t b, UI$BIT_t ans[2])
{
	UI$hBIT_t c[2];
	UI$hBIT_t d[2];
	UI$hBIT_t e[2];
	UI$hBIT_t f[2];
	UI$BIT_t tmp[2];

	if (!ans)
		ans = tmp;

	UI$hBIT_Add(a.L, b.L, c);
	UI$hBIT_Add(a.H, b.H, d);
	UI$hBIT_Add(d[0], c[1], e);
	UI$hBIT_Add(d[1], e[1], f);

	ans[0].L = c[0];
	ans[0].H = e[0];
	ans[1].L = f[0];
	ans[1].H = UI$hBIT_0();

	return ans[0];
}
UI$BIT_t UI$BIT_Sub(UI$BIT_t a, UI$BIT_t b)
{
	UI$BIT_t tmp[2];

	b = UI$BIT_Inv(b);
	b = UI$BIT_Add(b, UI$BIT_x(1), tmp);

	return UI$BIT_Add(a, b, tmp);
}
UI$BIT_t UI$BIT_Mul(UI$BIT_t a, UI$BIT_t b, UI$BIT_t ans[2])
{
	UI$hBIT_t c[2];
	UI$hBIT_t d[2];
	UI$hBIT_t e[2];
	UI$hBIT_t f[2];
	UI$hBIT_t m[5][5];
	uint mc[5] = { 0 }; // max are 1, 3, 5, 5, 4
	uint i;
	UI$BIT_t tmp[2];

	if (!ans)
		ans = tmp;

	UI$hBIT_Mul(a.L, b.L, c);
	UI$hBIT_Mul(a.L, b.H, d);
	UI$hBIT_Mul(a.H, b.L, e);
	UI$hBIT_Mul(a.H, b.H, f);

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
		UI$hBIT_t t = m[i][--mc[i]];
		UI$hBIT_t u = m[i][--mc[i]];
		UI$hBIT_t v[2];

		m[i][mc[i]++] = UI$hBIT_Add(t, u, v);
		m[i + 1][mc[i + 1]++] = v[1];
	}

	ans[0].L = m[0][0];
	ans[0].H = m[1][0];
	ans[1].L = m[2][0];
	ans[1].H = m[3][0];

	return ans[0];
}
static UI$BIT_t DivSub(UI$BIT_t a, UI$BIT_t b, UI$BIT_t ans[2], uint reti)
{
	UI$BIT_t tmp[2];

	errorCase(UI$BIT_IsZero(b));

	if (!ans)
		ans = tmp;

	ans[0] = UI$BIT_0();

	if (UI$hBIT_IsZero(b.H))
	{
		{
			UI$BIT_t dd;

			dd.L = UI$hBIT_0();
			dd.H = UI$hBIT_Div(a.H, b.L, NULL);

			ans[0] = UI$BIT_Add(ans[0], dd, NULL);
			a = UI$BIT_Sub(a, UI$BIT_Mul(b, dd, NULL));
		}

		while (!UI$hBIT_IsZero(a.H))
		{
			UI$BIT_t dd;
			UI$hBIT_t d2[2];

			UI$hBIT_Mul(UI$hBIT_Div(UI$hBIT_Fill(), b.L, NULL), a.H, d2);

			dd.L = d2[0];
			dd.H = d2[1];

			ans[0] = UI$BIT_Add(ans[0], dd, NULL);
			a = UI$BIT_Sub(a, UI$BIT_Mul(b, dd, NULL));
		}

		{
			UI$BIT_t dd;

			dd.L = UI$hBIT_Div(a.L, b.L, NULL);
			dd.H = UI$hBIT_0();

			ans[0] = UI$BIT_Add(ans[0], dd, NULL);
			a = UI$BIT_Sub(a, UI$BIT_Mul(b, dd, NULL));
		}
	}
	else
	{
		if (!UI$hBIT_IsFill(b.H))
		{
			UI$hBIT_t c = UI$hBIT_Add(b.H, UI$hBIT_x(1), NULL);

			for (; ; )
			{
				UI$hBIT_t d = UI$hBIT_Div(a.H, c, NULL);
				UI$BIT_t dd;

				if (UI$hBIT_IsZero(d))
					break;

				dd.L = d;
				dd.H = UI$hBIT_0();

				ans[0] = UI$BIT_Add(ans[0], dd, NULL);
				a = UI$BIT_Sub(a, UI$BIT_Mul(b, dd, NULL));
			}
		}
		while (0 <= UI$BIT_Comp(a, b))
		{
			ans[0] = UI$BIT_Add(ans[0], UI$BIT_x(1), NULL);
			a = UI$BIT_Sub(a, b);
		}
	}
	ans[1] = a;
	return ans[reti];
}
UI$BIT_t UI$BIT_Div(UI$BIT_t a, UI$BIT_t b, UI$BIT_t ans[2])
{
	return DivSub(a, b, ans, 0);
}
UI$BIT_t UI$BIT_Mod(UI$BIT_t a, UI$BIT_t b, UI$BIT_t ans[2])
{
	return DivSub(a, b, ans, 1);
}

int UI$BIT_IsZero(UI$BIT_t a)
{
	return UI$hBIT_IsZero(a.L) && UI$hBIT_IsZero(a.H);
}
int UI$BIT_IsFill(UI$BIT_t a)
{
	return UI$hBIT_IsFill(a.L) && UI$hBIT_IsFill(a.H);
}
sint UI$BIT_Comp(UI$BIT_t a, UI$BIT_t b)
{
	sint ret = UI$hBIT_Comp(a.H, b.H);

	if (!ret)
		ret = UI$hBIT_Comp(a.L, b.L);

	return ret;
}
UI$BIT_t UI$BIT_Fill(void)
{
	UI$BIT_t ans;

	ans.L = UI$hBIT_Fill();
	ans.H = UI$hBIT_Fill();

	return ans;
}

UI$BIT_t UI$BIT_DivTwo_Rem(UI$BIT_t a, uint *rem)
{
	UI$BIT_t ans;

	ans.H = UI$hBIT_DivTwo_Rem(a.H, rem);
	ans.L = UI$hBIT_DivTwo_Rem(a.L, rem);

	return ans;
}
UI$BIT_t UI$BIT_DivTwo(UI$BIT_t a)
{
	uint rem = 0;

	return UI$BIT_DivTwo_Rem(a, &rem);
}
