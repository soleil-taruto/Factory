#pragma once

#include "C:\Factory\Common\all.h"

/*
	pub_member
*/
typedef struct XNode_st
{
	char *Name; // このノードの名前
	char *Text; // このノードの値
	autoList_t *Children; // (XNode_t *)のリスト
}
XNode_t;

void NormalizeXNode(XNode_t *root, char *(*strFltr)(char *));
void ReleaseXNode(XNode_t *root);

// ---- accessor ----

XNode_t *GetDummyXNode(void);
autoList_t *CollectXNode(XNode_t *root, char *path);
XNode_t *GetXNode(XNode_t *root, char *path);
XNode_t *RefXNode(XNode_t *root, char *path);
XNode_t *ne_GetXNode(XNode_t *root, char *path);

// ----
