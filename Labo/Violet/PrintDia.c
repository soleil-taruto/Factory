// use int

#include "C:\Factory\Common\all.h"

static void PrintRep(int chr, int n)
{
	for (; n; n--)
		cout("%c", chr);
}
int main(int argc, char **argv)
{
	int s = atoi(nextArg());
	int c;

	errorCase(!m_isRange(s, 0, IMAX));

	for (c = -s; c <= s; c++)
	{
		PrintRep(' ', abs(c));
		PrintRep('*', (s - abs(c)) * 2 + 1);
		cout("\n");
	}
}
