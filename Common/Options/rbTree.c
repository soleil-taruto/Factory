/*
	既存のキーの値を再追加せずに変更する。
		rbtHasKey(tree, key);
		rbtJumpToLastAccess(tree);
		ReleaseValue(rbtGetCurrValue(tree)); 必要ならば開放
		rbtSetCurrValue(tree, newValue);

	全件走査する。
		rbtCallAllValue() を参照せよ。
*/

#include "rbTree.h"

static rbtElement_t *CreateElement(uint key, uint value, rbTree_t *tree)
{
	rbtElement_t *element = (rbtElement_t *)memAlloc(sizeof(rbtElement_t));

	element->Key = tree->FuncDuplexKey(key);
	element->Value = value;
	element->Links[0] = NULL;
	element->Links[1] = NULL;

	return element;
}
static sint CompElement(rbtElement_t *e1, rbtElement_t *e2, rbTree_t *tree)
{
	return tree->FuncCompKey(e1->Key, e2->Key);
}
static void ReleaseElement(rbtElement_t *element, rbTree_t *tree)
{
	tree->FuncReleaseKey(element->Key);
	memFree(element);
}

rbTree_t *rbCreateTree(uint (*funcDuplexKey)(uint), sint (*funcCompKey)(uint, uint), void (*funcReleaseKey)(uint))
{
	rbTree_t *tree = (rbTree_t *)memAlloc(sizeof(rbTree_t));

	errorCase(!funcDuplexKey);
	errorCase(!funcCompKey);
	errorCase(!funcReleaseKey);

	tree->Tree = rbtCreateTree((sint (*)(void *, void *, void *))CompElement, (void (*)(void *, void *))ReleaseElement, tree);
	tree->FuncDuplexKey = funcDuplexKey;
	tree->FuncCompKey = funcCompKey;
	tree->FuncReleaseKey = funcReleaseKey;
	tree->Count = 0;
	tree->CurrElement = NULL;

	return tree;
}
void rbReleaseTree(rbTree_t *tree)
{
	errorCase(!tree);

	rbtReleaseTree(tree->Tree);
	memFree(tree);
}

void rbtAddValue(rbTree_t *tree, uint key, uint value)
{
	rbtElement_t *element;

	errorCase(!tree);
	element = CreateElement(key, value, tree);

	// Links の更新、追加
	{
		rbtNode_t *root = tree->Tree->Root;

		if (root)
		{
			rbtElement_t *eRoot = (rbtElement_t *)root->Element;

			element->Links[0] = eRoot;
			element->Links[1] = eRoot->Links[1];

			eRoot->Links[1]->Links[0] = element;
			eRoot->Links[1] = element;
		}
		else
		{
			element->Links[0] = element;
			element->Links[1] = element;
		}
	}

	rbtAddElement(tree->Tree, element);
	tree->Count++;
}

static rbtElement_t *GetTarget(uint key)
{
	static rbtElement_t element;
	element.Key = key;
	return &element;
}
static int HasKey(rbTree_t *tree, rbtElement_t *target)
{
	errorCase(!tree);
	return rbtHasElement(tree->Tree, target);
}
int rbtHasKey(rbTree_t *tree, uint key)
{
	return HasKey(tree, GetTarget(key));
}
int rbtHasLastAccessKey(rbTree_t *tree)
{
	return HasKey(tree, NULL);
}

static uint GetValue(rbTree_t *tree, rbtElement_t *target)
{
	errorCase(!tree);
	return ((rbtElement_t *)rbtGetElement(tree->Tree, target))->Value;
}
uint rbtGetValue(rbTree_t *tree, uint key)
{
	return GetValue(tree, GetTarget(key));
}
uint rbtGetLastAccessValue(rbTree_t *tree)
{
	return GetValue(tree, NULL);
}

static uint UnaddValue(rbTree_t *tree, rbtElement_t *target)
{
	rbtElement_t *element;
	uint value;

	errorCase(!tree);

	element = (rbtElement_t *)rbtGetElement(tree->Tree, target);
	value = element->Value;

	// Links の更新、削除
	{
		element->Links[0]->Links[1] = element->Links[1];
		element->Links[1]->Links[0] = element->Links[0];

		if (tree->CurrElement == element)
		{
			tree->CurrElement = NULL;
		}
	}

	rbtRemoveElement(tree->Tree, NULL);
	tree->Count--;

	return value;
}
uint rbtUnaddValue(rbTree_t *tree, uint key)
{
	return UnaddValue(tree, GetTarget(key));
}
uint rbtUnaddLastAccessValue(rbTree_t *tree)
{
	return UnaddValue(tree, NULL);
}

uint rbtGetCount(rbTree_t *tree)
{
	errorCase(!tree);
	return tree->Count;
}

void rbtJump(rbTree_t *tree, uint direct) // direct == 0: Left, 1: Right
{
	rbtNode_t *node;

	errorCase(!tree);
	errorCase(!tree->CurrElement);
	errorCase(direct != 0 && direct != 1);

	tree->CurrElement = tree->CurrElement->Links[direct];
}
void rbtJumpForLeft(rbTree_t *tree)
{
	rbtJump(tree, 0);
}
void rbtJumpForRight(rbTree_t *tree)
{
	rbtJump(tree, 1);
}
void rbtJumpToLastAccess(rbTree_t *tree)
{
	errorCase(!tree);
	tree->CurrElement = (rbtElement_t *)rbtGetElement(tree->Tree, NULL);
}

static rbtElement_t *GetCurrElement(rbTree_t *tree)
{
	errorCase(!tree);
	errorCase(!tree->CurrElement);

	return tree->CurrElement;
}
uint rbtGetCurrKey(rbTree_t *tree)
{
	return GetCurrElement(tree)->Key;
}
uint rbtGetCurrValue(rbTree_t *tree)
{
	return GetCurrElement(tree)->Value;
}
void rbtSetCurrValue(rbTree_t *tree, uint value)
{
	GetCurrElement(tree)->Value = value;
}

void rbtCallAllValue(rbTree_t *tree, void (*funcOfValue)(uint))
{
	errorCase(!tree);
	errorCase(!funcOfValue);

	if (rbtGetCount(tree))
	{
		uint cnt;

		rbtSeekRoot(tree);
		rbtJumpToLastAccess(tree);

		for (cnt = rbtGetCount(tree); cnt; cnt--)
		{
			funcOfValue(rbtGetCurrValue(tree));
			rbtJumpForLeft(tree);
		}
	}
}
