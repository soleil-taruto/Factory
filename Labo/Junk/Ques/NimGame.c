/*
	三つの団子の山から、交互に団子を取る。
	一つの山から団子を取ります。取れなくなったら負けです。
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\bitList.h"

#define TALLLMT 100

/*
	[a][b][c] : a <= b <= c のみ使用

	0 - 未定義
	1 - 負ける
	2 - 勝てる
*/
static uchar WinCube[TALLLMT][TALLLMT][TALLLMT];

static int IsWin(uint a, uint b, uint c)
{
	uint win;

	/*
		a <= b <= c
	*/
	if (c < a) m_swap(c, a, uint);
	if (c < b) m_swap(c, b, uint);
	if (b < a) m_swap(b, a, uint);

	win = WinCube[a][b][c];

	if (!win)
	{
		uint n;

		win = 2;

		for (n = 0; n < a; n++) if (!IsWin(n, b, c)) goto foundNoWin;
		for (n = 0; n < b; n++) if (!IsWin(a, n, c)) goto foundNoWin;
		for (n = 0; n < c; n++) if (!IsWin(a, b, n)) goto foundNoWin;

		win = 1;
foundNoWin:
		WinCube[a][b][c] = win;
	}
	return win == 2;
}
int main(int argc, char **argv)
{
	uint a;
	uint b;
	uint c;

	WinCube[0][0][0] = 1;

	/*
		負ける組み合わせを表示する。
		a <= b <= c
	*/
	for (a = 1; a < TALLLMT; a++)
	for (b = a; b < TALLLMT; b++)
	for (c = b; c < TALLLMT; c++)
	{
		if (!IsWin(a, b, c))
			cout("%2u %2u %2u\n", a, b, c);
	}
}
