/*
	PrimeRange.exe [/P] [MIN-VAL MAX-VAL]

		/P               ... �f���̂ݕ\������B
		MIN-VAL, MAX-VAL ... �\���͈́A�ȗ������ 1 �` 2^64-2 �ɂȂ�BMAX-VAL �� 0 �ɂ���� 2^64-2 �ɂȂ�B

	���Ƃ��ǂ��]�v�ȕW���o�͂����邱�Ƃɒ��ӁI
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Prime2.h"

static int PrintPrimeOnlyFlag;

static void PrintIsPrime(uint64 value)
{
	if (PrintPrimeOnlyFlag)
	{
		if (IsPrime_R(value))
			cout("%I64u\n", value);
	}
	else
	{
		cout("%I64u %s prime.\n", value, IsPrime_R(value) ? "is" : "is not");
	}
}
static void PrimeRange(uint64 minval, uint64 maxval)
{
	uint64 count;

	if (maxval == 0)
		maxval = UINT64MAX - 1;

	errorCase(maxval < minval);
	errorCase(maxval == UINT64MAX);

	for (count = minval; count <= maxval; count++)
	{
		PrintIsPrime(count);
	}
}
int main(int argc, char **argv)
{
	if (argIs("/P"))
	{
		PrintPrimeOnlyFlag = 1;
	}
	if (hasArgs(2))
	{
		uint64 minval;
		uint64 maxval;

		minval = toValue64(nextArg());
		maxval = toValue64(nextArg());

		PrimeRange(minval, maxval);
		return;
	}
	PrimeRange(2, UINT64MAX - 1);
}
