#pragma once

#include "C:\Factory\Common\all.h"

typedef struct VTree_st
{
	uint (*GetLocalCount)(void);   // �J�����g�f�B���N�g���̃f�B���N�g���E�t�@�C��������Ԃ��B
	char *(*GetLocal)(uint index); // ���[�J���t�@�C������Ԃ��B�������߂�l�͌Ăяo�����ŊJ�����邱�Ɓ�����
	int (*IsDir)(uint index);      // �f�B���N�g���ł��邩�ǂ����Ԃ��B
	void (*IntoDir)(uint index);   // �f�B���N�g���ɓ���B
	void (*ExitDir)(void);         // �e�f�B���N�g���ֈړ�����B
	uint64 (*GetSize)(uint index); // �t�@�C���̃T�C�Y��Ԃ��B
	void (*GetEntity)(uint index, uint64 startPos, uint size, void *outBuff); // �t�@�C���̕�����Ԃ��B
}
VTree_t;
