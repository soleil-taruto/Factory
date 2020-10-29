#include "IsPrime.h"

int UseMillerRabinTestMode;

int A_IsPrime(uint64 value)
{
	return ( UseMillerRabinTestMode ? IsPrime_M : IsPrime )(value);
}
int A_IsPrime_R(uint64 value)
{
	return IsPrime_R(value);
}
