#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	int i;

	for (i = 1; i <= 10; i++)
	{
		int s = i * 1000000;
		int z = s / 10 * i;

		cout("%d - %d = %d\n", s, z, s - z);
	}
}
