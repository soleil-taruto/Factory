/*
	9 / 10 = 1 / a + 1 / b + 1 / c
*/

#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	int a;
	int b;
	int c;

	for (a = 1; a < 100; a++)
	for (b = 1; b < a; b++)
	for (c = 1; c < b; c++)
	{
		int n = a * b + b * c + c * a;
		int d = a * b * c;

		n *= 10;
		d *= 9;

		if (n == d)
		{
			cout("%d %d %d\n", a, b, c);
		}
	}
}
