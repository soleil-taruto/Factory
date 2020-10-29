#pragma once

#include "C:\Factory\Common\all.h"

typedef struct rbtNode_st // static member // “à•”‚Å‚Ì‚ÝŽg—p
{
	void *Element;
	int Red;
	struct rbtNode_st *Parent;
	struct rbtNode_st *Children[2];
}
rbtNode_t;

typedef struct rbtTree_st // static member
{
	sint (*FuncComp)(void *, void *, void *);
	void (*FuncRelease)(void *, void *);
	void *ExtraData;
	rbtNode_t *Root;
	rbtNode_t *LastAccessNode;
}
rbtTree_t;

rbtTree_t *rbtCreateTree(sint (*funcComp)(void *, void *, void *), void (*funcRelease)(void *, void *), void *extraData);
void rbtReleaseTree(rbtTree_t *tree);

void rbtAddElement(rbtTree_t *tree, void *element);
int rbtHasElement(rbtTree_t *tree, void *element);
void *rbtGetElement(rbtTree_t *tree, void *element);
void rbtRemoveElement(rbtTree_t *tree, void *element);

void rbtSeekRootElement(rbtTree_t *tree);
void rbtSeekForEndElement(rbtTree_t *tree, uint direct);
void rbtSeekForLeftEndElement(rbtTree_t *tree);
void rbtSeekForRightEndElement(rbtTree_t *tree);

void rbtSeekElement(rbtTree_t *tree, uint direct);
void rbtSeekForLeftElement(rbtTree_t *tree);
void rbtSeekForRightElement(rbtTree_t *tree);
