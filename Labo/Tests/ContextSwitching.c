#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Thread.h"

static void TestFunc(uint prm) // ts_
{
	uint c;

	critical();
	{
//		Sleep(100); // 全員ここまで来るのを待つ。

		for (c = 0; c < 100; c++)
		{
			cout("%s:%u\n", prm, c);

			inner_uncritical();
			{
				// noop -- ここでスレッドの切り替えが起きて欲しい。
			}
			inner_critical();
		}
	}
	uncritical();
}
int main(int argc, char **argv)
{
	uint ths[3];

	critical();
	{
		ths[0] = runThread(TestFunc, (uint)"Th_0");
		ths[1] = runThread(TestFunc, (uint)"Th_1");
		ths[2] = runThread(TestFunc, (uint)"Th_2");
	}
	uncritical();

	TestFunc((uint)"Th_M");

	waitThread(ths[0]);
	waitThread(ths[1]);
	waitThread(ths[2]);
}
