#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\CRRandom.h"
#include "C:\Factory\Common\Options\uintx\uint64.h"

static uint64 GetTestValue(void)
{
	switch (mt19937_rnd(4))
	{
	case 0: return mt19937_rnd64Mod((uint64)30000);
	case 1: return mt19937_rnd64Mod((uint64)IMAX);
	case 2: return mt19937_rnd64Mod((uint64)IMAX * 30000);
	case 3: return mt19937_rnd64Mod(IMAX_64);

	default:
		error(); // never
	}
	error(); // never
	return 0ui64; // dummy
}
static void Test01(void)
{
	while (!waitKey(0))
	{
		{
			uint64 a = GetTestValue();
			uint64 b = GetTestValue();
			uint64 c;
			UI64_t ta;
			UI64_t tb;
			UI64_t tc;
			uint64 cc;

			c = a + b;

			cout("%I64u + %I64u = %I64u\n", a, b, c);

			ta.L.Value = (uint)a;
			ta.H.Value = (uint)(a >> 32);
			tb.L.Value = (uint)b;
			tb.H.Value = (uint)(b >> 32);

			tc = UI64_Add(ta, tb, NULL);

			cc = (uint64)tc.H.Value << 32 | tc.L.Value;

			errorCase(c != cc);
		}

		{
			uint64 a = GetTestValue();
			uint64 b = GetTestValue();
			uint64 c;
			UI64_t ta;
			UI64_t tb;
			UI64_t tc;
			uint64 cc;

#if 1
			c = a < b ? UINT64MAX - b + a + 1UI64 : a - b;
#else
			c = a - b; // オーバーフローする。
#endif

			cout("%I64u - %I64u = %I64u\n", a, b, c);

			ta.L.Value = (uint)a;
			ta.H.Value = (uint)(a >> 32);
			tb.L.Value = (uint)b;
			tb.H.Value = (uint)(b >> 32);

			tc = UI64_Sub(ta, tb);

			cc = (uint64)tc.H.Value << 32 | tc.L.Value;

			errorCase(c != cc);
		}

		{
			uint64 a = GetTestValue();
			uint64 b = GetTestValue();
			uint64 c;
			UI64_t ta;
			UI64_t tb;
			UI64_t tc;
			uint64 cc;

			if (a != 0 && UINT64MAX / a < b)
				b = mt19937_rnd64Mod(UINT64MAX / a);

			c = a * b;

			cout("%I64u * %I64u = %I64u\n", a, b, c);

			ta.L.Value = (uint)a;
			ta.H.Value = (uint)(a >> 32);
			tb.L.Value = (uint)b;
			tb.H.Value = (uint)(b >> 32);

			tc = UI64_Mul(ta, tb, NULL);

			cc = (uint64)tc.H.Value << 32 | tc.L.Value;

			errorCase(c != cc);
		}

		{
			uint64 a = GetTestValue();
			uint64 b = GetTestValue();
			uint64 c;
			UI64_t ta;
			UI64_t tb;
			UI64_t tc;
			uint64 cc;

			m_maxim(b, 1);

			c = a / b;

			cout("%I64u / %I64u = %I64u\n", a, b, c);

			ta.L.Value = (uint)a;
			ta.H.Value = (uint)(a >> 32);
			tb.L.Value = (uint)b;
			tb.H.Value = (uint)(b >> 32);

			tc = UI64_Div(ta, tb, NULL);

			cc = (uint64)tc.H.Value << 32 | tc.L.Value;

			errorCase(c != cc);
		}

		{
			uint64 a = GetTestValue();
			uint64 b = GetTestValue();
			uint64 c;
			UI64_t ta;
			UI64_t tb;
			UI64_t tc;
			uint64 cc;

			m_maxim(b, 1);

			c = a % b;

			cout("%I64u %% %I64u = %I64u\n", a, b, c);

			ta.L.Value = (uint)a;
			ta.H.Value = (uint)(a >> 32);
			tb.L.Value = (uint)b;
			tb.H.Value = (uint)(b >> 32);

			tc = UI64_Mod(ta, tb, NULL);

			cc = (uint64)tc.H.Value << 32 | tc.L.Value;

			errorCase(c != cc);
		}
	}
	cout("Stopped\n");
}
int main(int argc, char **argv)
{
	mt19937_initCRnd();

	Test01();
}
