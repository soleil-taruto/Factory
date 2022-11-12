#pragma comment(lib, "user32.lib")

#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
    POINT pos;
	uint lastX = 0;
	uint lastY = 0;
	uint slpcnt = 0;

	while (!hasKey() || getKey() != 0x1b)
	{
		uint x;
		uint y;

		GetCursorPos(&pos);

		x = (uint)pos.x;
		y = (uint)pos.y;

		if (lastX == x && lastY == y) // ? not moved
		{
			if (slpcnt < 300)
				slpcnt++;
		}
		else // ? moved
		{
			cout("%u, %u\n", x, y);

			lastX = x;
			lastY = y;
			slpcnt = 0;
		}
		sleep(slpcnt);
	}
}
