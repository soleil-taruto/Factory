#pragma comment(lib, "user32.lib")

#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	int swpbtn = GetSystemMetrics(SM_SWAPBUTTON);

	while (!hasKey())
	{
		int lbtn = GetAsyncKeyState(VK_LBUTTON) ? 1 : 0;
		int rbtn = GetAsyncKeyState(VK_RBUTTON) ? 1 : 0;

		if (swpbtn)
		{
			m_swap(lbtn, rbtn, int);
		}

		cout("%c%c\n", lbtn ? 'L' : '-', rbtn ? 'R' : '-');

		sleep(10);
	}
}
