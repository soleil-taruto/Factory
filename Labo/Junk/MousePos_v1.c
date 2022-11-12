#pragma comment(lib, "user32.lib")

#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
    POINT pos;

	while (!hasKey() || getKey() != 0x1b)
	{
	    GetCursorPos(&pos);
		cout("%d, %d\n", pos.x, pos.y);
		sleep(100);
	}
}
