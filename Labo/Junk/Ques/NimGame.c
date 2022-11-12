/*
	�O�̒c�q�̎R����A���݂ɒc�q�����B
	��̎R����c�q�����܂��B���Ȃ��Ȃ����畉���ł��B
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\bitList.h"

#define TALLLMT 100

/*
	[a][b][c] : a <= b <= c �̂ݎg�p

	0 - ����`
	1 - ������
	2 - ���Ă�
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
		������g�ݍ��킹��\������B
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
