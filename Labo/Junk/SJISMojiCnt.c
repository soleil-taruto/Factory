/*
	7724 •¶Žš
*/

#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	int n = 0;
	int c;

	for (c = 0x0000; c <= 0xffff; c++)
		if (isJChar(c))
			n++;

	cout("%d\n", n);
}
