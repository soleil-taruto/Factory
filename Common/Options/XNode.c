#include "XNode.h"

static char *DefStrFltr(char *str)
{
	line2JLine(str, 1, 1, 1, 1);
	return str;
}
static char *StrFltrWrap(char *str, char *(*strFltr)(char *))
{
	if (!str)
		str = strx("");

	return strFltr(str);
}
void NormalizeXNode(XNode_t *root, char *(*strFltr)(char *))
{
	autoList_t *nodes = newList();

	errorCase(!root);

	if (!strFltr)
		strFltr = DefStrFltr;

	addElement(nodes, (uint)root);

	while (getCount(nodes))
	{
		XNode_t *node = (XNode_t *)unaddElement(nodes);

		node->Name = StrFltrWrap(node->Name, strFltr);
		node->Text = StrFltrWrap(node->Text, strFltr);

		if (!node->Children)
			node->Children = newList();

		removeZero(node->Children);
		addElements(nodes, node->Children);
	}
	releaseAutoList(nodes);
}
void ReleaseXNode(XNode_t *root)
{
	if (!root)
		return;

	memFree(root->Name);
	memFree(root->Text);

	if (root->Children)
		callAllElement_x(root->Children, (void (*)(uint))ReleaseXNode);

	memFree(root);
}

// ---- accessor ----

XNode_t *GetDummyXNode(void)
{
	static XNode_t *node;

	if (!node)
	{
		node = (XNode_t *)memAlloc(sizeof(XNode_t));

		node->Name = strx("Dummy");
		node->Text = strx("Dummy");
		node->Children = newList();
	}
	return node;
}

static autoList_t *CXN_PTkns;
static autoList_t *CXN_Dest;

static void CXN_Main(XNode_t *root, uint pTknIndex)
{
	if (pTknIndex < getCount(CXN_PTkns))
	{
		XNode_t *node;
		uint index;

		foreach (root->Children, node, index)
			if (!strcmp(node->Name, getLine(CXN_PTkns, pTknIndex)))
				CXN_Main(node, pTknIndex + 1);
	}
	else
	{
		addElement(CXN_Dest, (uint)root);
	}
}
autoList_t *CollectXNode(XNode_t *root, char *path)
{
	errorCase(!root);
	errorCase(!path);

	CXN_PTkns = tokenize(path, '/');
	CXN_Dest = newList();

	// 空のpTknをWarning
	{
		char *pTkn;
		uint index;

		foreach (CXN_PTkns, pTkn, index)
			if (!*pTkn)
				cout("Warning: XNodeパスに空のパストークンが含まれています。\n");
	}

	CXN_Main(root, 0);

	releaseDim(CXN_PTkns, 1);
	return CXN_Dest;
}
XNode_t *GetXNode(XNode_t *root, char *path)
{
	autoList_t *nodes = CollectXNode(root, path);
	XNode_t *node;

	if (getCount(nodes))
		node = (XNode_t *)getElement(nodes, 0);
	else
		node = NULL;

	releaseAutoList(nodes);
	return node;
}
XNode_t *RefXNode(XNode_t *root, char *path)
{
	XNode_t *node = GetXNode(root, path);

	if (!node)
		node = GetDummyXNode();

	return node;
}
XNode_t *ne_GetXNode(XNode_t *root, char *path)
{
	XNode_t *node = GetXNode(root, path);

	errorCase(!node);
	return node;
}

// ----
