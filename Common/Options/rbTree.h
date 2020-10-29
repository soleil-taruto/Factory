#pragma once

#include "C:\Factory\Common\all.h"
#include "rbt.h"

typedef struct rbtElement_st // static member // “à•”‚Å‚Ì‚ÝŽg—p
{
	uint Key;
	uint Value;
	struct rbtElement_st *Links[2];
}
rbtElement_t;

typedef struct rbTree_st // static member
{
	rbtTree_t *Tree;
	uint (*FuncDuplexKey)(uint);
	sint (*FuncCompKey)(uint, uint);
	void (*FuncReleaseKey)(uint);
	uint Count;
	rbtElement_t *CurrElement;
}
rbTree_t;

rbTree_t *rbCreateTree(uint (*funcDuplexKey)(uint), sint (*funcCompKey)(uint, uint), void (*funcReleaseKey)(uint));
void rbReleaseTree(rbTree_t *tree);

void rbtAddValue(rbTree_t *tree, uint key, uint value);

int rbtHasKey(rbTree_t *tree, uint key);
int rbtHasLastAccessKey(rbTree_t *tree);

uint rbtGetValue(rbTree_t *tree, uint key);
uint rbtGetLastAccessValue(rbTree_t *tree);

uint rbtUnaddValue(rbTree_t *tree, uint key);
uint rbtUnaddLastAccessValue(rbTree_t *tree);

uint rbtGetCount(rbTree_t *tree);

void rbtJump(rbTree_t *tree, uint direct);
void rbtJumpForLeft(rbTree_t *tree);
void rbtJumpForRight(rbTree_t *tree);
void rbtJumpToLastAccess(rbTree_t *tree);

uint rbtGetCurrKey(rbTree_t *tree);
uint rbtGetCurrValue(rbTree_t *tree);
void rbtSetCurrValue(rbTree_t *tree, uint value);

#define rbtSeekRoot(tree) \
	rbtSeekRootElement((tree)->Tree)
#define rbtSeekForEnd(tree, direct) \
	rbtSeekForEndElement((tree)->Tree, (direct))
#define rbtSeekForLeftEnd(tree) \
	rbtSeekForLeftEndElement((tree)->Tree)
#define rbtSeekForRightEnd(tree) \
	rbtSeekForRightEndElement((tree)->Tree)

#define rbtSeek(tree, direct) \
	rbtSeekElement((tree)->Tree, (direct))
#define rbtSeekForLeft(tree) \
	rbtSeekForLeftElement((tree)->Tree)
#define rbtSeekForRight(tree) \
	rbtSeekForRightElement((tree)->Tree)

void rbtCallAllValue(rbTree_t *tree, void (*funcOfValue)(uint));
