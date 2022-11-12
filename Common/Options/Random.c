#include "Random.h"

/*
	block:
		b[0] ... [n]

	ret:
		b[0] ... [n] +
		sha512(b[0] ... b[n + 64 * 0])[0] ... [63] +
		sha512(b[0] ... b[n + 64 * 1])[0] ... [63] +
		sha512(b[0] ... b[n + 64 * 2])[0] ... [63] +
		sha512(b[0] ... b[n + 64 * 3])[0] ... [63] +
		...
		sha512(b[0] ... b[n + 64 * (exnum - 1)])[0] ... [63]
*/
void sha512_expand(autoBlock_t *block, uint exnum) // exnum: 0- (0 as noop)
{
	sha512_t *ctx = sha512_create();
	uint index;
	autoBlock_t gab;

	sha512_update(ctx, block);

	for (index = 0; index < exnum; index++)
	{
		if (index)
			sha512_update(ctx, gndBlockVar(sha512_hash, 64, gab));

		sha512_makeHash(ctx);
		ab_addBlock(block, sha512_hash, 64);
	}
}
void mt19937_initRnd(uint seed)
{
	autoBlock_t *initKey = newBlock();

	ab_addValue(initKey, seed);
	sha512_expand(initKey, 39);

	// (seed-size + (sha512-size * 39)) / uint-size == (4 + (64 * 39)) / 4 == 625 == N + 1

	mt19937_initByArray(initKey);
	releaseAutoBlock(initKey);
}
uint getRandElement(autoList_t *list)
{
	return getElement(list, mt19937_rnd(getCount(list)));
}
