/*
	乱数列に同じ乱数列をXORすると全部ゼロになるが、
	乱数列に同じ乱数列をずらして(Rotateして)XORするのもマズいんじゃないか。
	-> マズいっぽい。

	- - -

	乱数列 b1, b2, b3, ... bn と bn, b1, b2, ... b(n-1) を XOR すると、// bx は N ビットの乱数列 (N == 1～)
	(b1 ^ bn), (b2 ^ b1), (b3 ^ b2), ... (bn, b(n-1))
	んで、
	(b1 ^ bn) ^ (b2 ^ b1) ^ (b3 ^ b2) ^ ... (b(n-1), b(n-2)) == (bn, b(n-1))
	なので x ビット ROT して XOR すると「最後の x ビット」以外を知っていれば「最後の x ビット」が分かる。

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
