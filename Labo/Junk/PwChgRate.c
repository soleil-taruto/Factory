/*
	�p�X���[�h�����蒆�A���̕p�x�i�m���j�ŕύX�����Ƃ��̔j����m��
	�ύX���Ȃ� -> 1.0
	�p�ɂɕύX����ق� 1-(1/e) �ɋ߂Â����ۂ��B
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Random.h"

#define LOOP_MAX 5
#define TEST_MAX 10000
#define PW_MAX 10000

static void DoTest(double pwChgRate)
{
	uint loop_cnt;

	cout("PW_CHG_RATE: %f\n", pwChgRate);
	cout("\n");

	for (loop_cnt = 0; loop_cnt < LOOP_MAX; loop_cnt++)
	{
		uint succeed_cnt = 0;
		uint test_cnt;

		for (test_cnt = 0; test_cnt < TEST_MAX; test_cnt++)
		{
			uint pw = mt19937_rnd(PW_MAX);
			uint attack;

			for (attack = 0; attack < PW_MAX; attack++)
			{
				double r = mt19937_rnd(IMAX) / (double)IMAX;

				if (r < pwChgRate)
					pw = mt19937_rnd(PW_MAX);

				if (attack == pw)
				{
					succeed_cnt++;
					break;
				}
			}
		}
		cout("%u / %u = %f\n", succeed_cnt, test_cnt, (double)succeed_cnt / test_cnt);
	}
	cout("\n");
}
int main(int argc, char **argv)
{
	mt19937_initRnd(time(NULL));

	DoTest(0.0);

	DoTest(0.0001);
	DoTest(0.0002);
	DoTest(0.0003);
	DoTest(0.0004);
	DoTest(0.0005);

	DoTest(0.001);
	DoTest(0.0012);
	DoTest(0.0013);
	DoTest(0.0014);
	DoTest(0.0015);

	DoTest(0.002);
	DoTest(0.003);
	DoTest(0.004);
	DoTest(0.005);

	DoTest(0.01);
	DoTest(0.03);

	DoTest(0.1);
	DoTest(0.3);
	DoTest(0.9);

	DoTest(0.95);
	DoTest(0.96);
	DoTest(0.97);
	DoTest(0.98);
	DoTest(0.99);

	DoTest(1.0);
}
