#pragma once

#include "C:\Factory\Common\all.h"

/*
	pub_member
*/
typedef struct XNode_st
{
	char *Name; // ���̃m�[�h�̖��O
	char *Text; // ���̃m�[�h�̒l
	autoList_t *Children; // (XNode_t *)�̃��X�g
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
