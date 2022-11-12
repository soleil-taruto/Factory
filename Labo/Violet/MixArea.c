/*

--

世界には n 種類の言語があるものとし、それぞれ g_1, g_2, ... g_n とする。


ある言語 g_X を話せる人の数を h(g_X) とする。

{ h(g_1), h(g_2), ... h(g_n) } の合計を i(1) とする。


少なくとも g_a1, g_a2, ... g_aY 全て話せる人の数を H(g_a1, g_a2, ... g_aY) とする。

あらゆる m 個の言語の組み合わせ k_1, k_2, ... k_Z について、※
{ H(k_1), H(k_2), ... H(k_Z) } の合計を i(m) とする。

※ n=3, m=2 の場合、{ (g_1, g_2), (g_1, g_3), (g_2, g_3) } の3つになる。


少なくとも1つの言語を話せる人の数は次の式で表せる。


n=1の場合

i(1)

n=2の場合

i(1) - i(2)

n=3の場合

i(1) - i(2) + i(3)

n=4の場合

i(1) - i(2) + i(3) - i(4)

n=奇数の場合

i(1) - i(2) + i(3) - i(4) + i(5) - ... + i(n - 2) - i(n - 1) + i(n)

n=偶数の場合

i(1) - i(2) + i(3) - i(4) + i(5) - ... - i(n - 2) + i(n - 1) - i(n)


-- というのが正しいかどうか確認するプログラム

*/

// use int

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\CRRandom.h"

#define N_MAX 16
#define T_MAX 100000

static int Map[N_MAX][T_MAX];   // [言語][人] == この人が、この言語を話せるか。
static int NN;                  // 言語の数
static int TT;                  // 人の数
static double RR;

static double GetDRand(void) // ret: 0.0 ～ 1.0
{
	return mt19937_rnd32() / (double)UINTMAX;
}
static void MkMap(void)
{
	int n;
	int t;

	for (n = 0; n < NN; n++)
	for (t = 0; t < TT; t++)
	{
		Map[n][t] = GetDRand() < RR;
	}
}
static void PrintMap(void)
{
	int n;
	int t;

	cout("---- Map ----\n");

	for (n = 0; n < NN; n++)
	{
		for (t = 0; t < TT; t++)
		{
			cout("%c", Map[n][t] ? '1' : '0');
		}
		cout("\n");
	}
	cout("----\n");
}
static int GetArea_1(void)
{
	int ret = 0;
	int n;
	int t;

	for (t = 0; t < TT; t++)
	for (n = 0; n < NN; n++)
	{
		if (Map[n][t])
		{
			ret++;
			break; // next t
		}
	}
	return ret;
}

static int ALst[N_MAX + 1];
static int NMsk[N_MAX];

static int GA_AtLeastNMsk(int t) // ret: ? Map[][t] について、NMsk[n] == 1 なら Map[n][t] == 1 である。
{
	int n;

	for (n = 0; n < NN; n++)
		if (NMsk[n] && !Map[n][t])
			return 0;

	return 1;
}
static void GA_Main(void)
{
	int n;
	int cnt = 0;
	int t;

	for (n = 0; n < NN; n++)
		if (NMsk[n])
			cnt++;

	for (t = 0; t < TT; t++)
		if (GA_AtLeastNMsk(t))
			ALst[cnt]++;
}
static int GA_Next(void)
{
	int n;

	for (n = 0; n < NN; n++)
	{
		if (!NMsk[n])
		{
			NMsk[n] = 1;
			return 1;
		}
		NMsk[n] = 0;
	}
	return 0;
}
static int GetArea_2(void)
{
	int ret;
	int n;

	zeroclear(ALst);
	zeroclear(NMsk);

	do
	{
		GA_Main();
	}
	while (GA_Next());

	ret = 0;

	for (n = 1; n <= NN; n++)
	{
		if (n % 2)
			ret += ALst[n];
		else
			ret -= ALst[n];
	}
	return ret;
}
static void DoTest_2(int n, int t, double r)
{
	int a1;
	int a2;

	LOGPOS();

	if (waitKey(0))
		termination(0);

	NN = n;
	TT = t;
	RR = r;

	cout("DoTest_2: %d %d %f\n", NN, TT, RR);

	MkMap();
//	PrintMap();

	a1 = GetArea_1();
	a2 = GetArea_2();

	cout("a1: %d\n", a1);
	cout("a2: %d\n", a2);

	errorCase(a1 != a2);

	LOGPOS();
}
static void DoTest(int n, int t)
{
	DoTest_2(n, t, 0.01);
	DoTest_2(n, t, 0.1);
	DoTest_2(n, t, 0.5);
	DoTest_2(n, t, 1.0 / n);
	DoTest_2(n, t, 1.0 / t);
	DoTest_2(n, t, 0.9);
	DoTest_2(n, t, 0.99);
}
int main(int argc, char **argv)
{
	int n;
	int t;

	mt19937_initCRnd();

	for (n = 1; n <= N_MAX; n++)
	for (t = 1; t <= T_MAX; t *= 10)
	{
		DoTest(n, t);
	}
	for (; ; )
	{
		DoTest_2(
			mt19937_rnd(N_MAX) + 1,
			mt19937_rnd(T_MAX) + 1,
			GetDRand()
			);
	}
}
