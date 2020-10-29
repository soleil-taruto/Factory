#include "CRRandom.h"

void mt19937_initCRnd(void)
{
	autoBlock_t *initKey = makeCryptoRandBlock(624 * 4); // 624 == N
	mt19937_initByArray(initKey);
	releaseAutoBlock(initKey);
}
