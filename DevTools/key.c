#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	int crtMode = argIs("/CRT");

	for (; ; )
	{
		int chr = crtMode ? _getch() : getKey();

		cout("%02x (%u)\n", chr, chr);

		if (chr == 0x1b)
			break;
	}
}
