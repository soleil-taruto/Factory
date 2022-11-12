#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	hasArgs(0); // for //x options

	for (; ; )
	{
		int chr = readChar(stdin);

		if (chr == EOF)
			break;

		cout("%c", chr);
	}
}
