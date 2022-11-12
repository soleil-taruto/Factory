/*
	Ç‹Ç∏Ç±Ç»Ç¢ÅB
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\CRandom.h"
#include "C:\Factory\Common\Options\rbTree.h"
#include "C:\Factory\OpenSource\sha512.h"

// ---- hashes_func ----

static uint HashClone(uint h)
{
	return (uint)memClone((void *)h, 64);
}
static sint CompHash(uint h1, uint h2)
{
	uchar *hash1 = (uchar *)h1;
	uchar *hash2 = (uchar *)h2;

	return memcmp(hash1, hash2, 64);
}
static void ReleaseHash(uint h)
{
	memFree((void *)h);
}

// ----

static void DoTest_H(uchar *hash)
{
	rbTree_t *hashes = rbCreateTree(HashClone, CompHash, ReleaseHash);
	uint count = 0;
	autoBlock_t gab;

	for (; ; )
	{
		if (count < 10000 || count % 10000 == 0)
			cout("[%u] %s\n", count, c_makeHexLine(gndBlockVar(hash, 64, gab)));

		if (rbtHasKey(hashes, (uint)hash))
		{
			cout("COLL @ %u\n", count);
			break;
		}
		rbtAddValue(hashes, (uint)hash, 0);

		/*
			next hash
		*/
		sha512_makeHashBlock(gndBlockVar(hash, 64, gab));
		hash = sha512_hash;

		count++;
	}
	rbReleaseTree(hashes);
}
static void DoTest(void *block, uint size)
{
	autoBlock_t gab;

	sha512_makeHashBlock(gndBlockVar(block, size, gab));

	DoTest_H(sha512_hash);
}
int main(int argc, char **argv)
{
	DoTest("a", 1);
}
