#include "all.h"

// ---- tree ----

static rbTree_t *Tree;

static void CreateTree(void)
{
	Tree = rbCreateTree((uint (*)(uint))strx, (sint (*)(uint, uint))strcmp, (void (*)(uint))memFree);
}
static void ReleaseTree(void)
{
	rbtCallAllValue(Tree, (void (*)(uint))ReleaseMvAvg);
	rbReleaseTree(Tree);
}
static void INIT(void)
{
	if (Tree) // ? already inited
		return;

	CreateTree();
}
static uint GetKey(uint64 span, char *pair)
{
	static char *key;

	memFree(key);
	key = xcout("%I64u_%s", span, pair);
	return (uint)key;
}

// ----

MvAvg_t *MAC_GetMvAvg(uint fxTime, uint span, char *pair)
{
	MvAvg_t *i;

	INIT();

	if (!rbtHasKey(Tree, GetKey(span, pair)))
	{
		i = CreateMvAvg(fxTime, span, pair); // ‚±‚Ì’†‚Å MAC_Clear() ‚ªŒÄ‚Î‚ê‚éê‡ƒAƒŠ
		rbtAddValue(Tree, GetKey(span, pair), (uint)i);
	}
	else
	{
		i = (MvAvg_t *)rbtGetLastAccessValue(Tree);
	}
	return i;
}
void MAC_Clear(void)
{
	INIT();

	ReleaseTree();
	CreateTree();
}
