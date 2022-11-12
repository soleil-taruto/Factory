/*
	������ɓ����������XOR����ƑS���[���ɂȂ邪�A
	������ɓ�������������炵��(Rotate����)XOR����̂��}�Y���񂶂�Ȃ����B
	-> �}�Y�����ۂ��B

	- - -

	������ b1, b2, b3, ... bn �� bn, b1, b2, ... b(n-1) �� XOR ����ƁA// bx �� N �r�b�g�̗����� (N == 1�`)
	(b1 ^ bn), (b2 ^ b1), (b3 ^ b2), ... (bn, b(n-1))
	��ŁA
	(b1 ^ bn) ^ (b2 ^ b1) ^ (b3 ^ b2) ^ ... (b(n-1), b(n-2)) == (bn, b(n-1))
	�Ȃ̂� x �r�b�g ROT ���� XOR ����Ɓu�Ō�� x �r�b�g�v�ȊO��m���Ă���΁u�Ō�� x �r�b�g�v��������B

	- - -
*/

#include "C:\Factory\Common\all.h"

//#define SHIFT_B 1
//#define SHIFT_B 2
//#define SHIFT_B 3
#define SHIFT_B 4

static uint Counters[0x10000];

static uint ShiftV(uint v)
{
	return ((v >> SHIFT_B) | (v << (16 - SHIFT_B))) & 0xffff;
}
static void PutV(uint v)
{
	v ^= ShiftV(v);

	Counters[v]++;
}
int main(int argc, char **argv)
{
	uint v;

	for (v = 0x0000; v <= 0xffff; v++)
	{
		PutV(v);
	}
	for (v = 0x0000; v <= 0xffff; v++)
	{
		cout("0x%04x <- %u\n", v, Counters[v]);
	}
}
