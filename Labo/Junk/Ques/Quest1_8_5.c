/*
	Quest 1-8-5 でループしない確率
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\CRRandom.h"

int main(int argc, char **argv)
{
	int madokami_sai;
	int c;

	mt19937_initCRnd();

	for (madokami_sai = 1; madokami_sai <= 7; madokami_sai++)
	{
		cout("madokami_sai: %d\n", madokami_sai); // 7 == 次のワープを選ぶ -> 必ずワープ

		for (c = 1; c <= 10; c++)
		{
			int n = 0;
			int d = 0;

			while (d < 1000000)
			{
				char *fld = "f---*-*----*";
				int pos = 0;

				for (; ; )
				{
					int sai = mt19937_range(1, 6);

					if (sai == 6) // ? まど神様
					{
						if (madokami_sai == 7)
						{
							n++;
							break;
						}
						sai = madokami_sai;
					}
					pos += sai;

					if (strlen(fld) <= pos)
						break;

					if (fld[pos] == '*')
					{
						n++;
						break;
					}
				}
				d++;
			}
			cout("%f = %d / %d\n", (double)n / d, n, d);
		}
		cout("\n");
	}
}
