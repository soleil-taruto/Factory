#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	for (; ; )
	{
		int key = getKey();

		cout("%02x\n", key);

		if (key == 0x1b)
			break;
	}
}
