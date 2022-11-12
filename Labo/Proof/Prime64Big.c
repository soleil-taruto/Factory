#include "C:\Factory\Common\all.h"

static int Test_IsPrime(uint64 value)
{
	uint64 denom;

	if (value < 2)
		return 0;

	if (value == 2)
		return 1;

	if (value % 2 == 0)
		return 0;

	for (denom = 3; denom <= UINTMAX; denom += 2)
	{
		if (value < denom * denom)
			break;

		if (value % denom == 0)
			return 0;
	}
	return 1;
}
int main(int argc, char **argv)
{
	uint64 value;

	for (value = UINT64MAX; UINT64MAX - 100 <= value; value--)
	{
		if (Test_IsPrime(value))
		{
			cout("%I64u\n", value);
		}
	}
}
