#include "GitCommon.h"

static int IsGitPath(char *path)
{
	return (int)mbs_stristr(path, "\\.git"); // .git �Ŏn�܂郍�[�J�������܂�
}
void RemoveGitPaths(autoList_t *paths)
{
	char *path;
	uint index;

	foreach (paths, path, index)
		if (IsGitPath(path))
			path[0] = '\0';

	trimLines(paths);
}
/*
	�}�X�N�������ʁA���p�s�\�ɂȂ������\�[�X�ɂ��Ė��O��ޔ�����B
	�ǂݍ��݃G���[�ɂȂ���A�t�@�C���������G���[�̕������p�s�\�ł��邱�Ƃ�������₷�����낤����B
	�\�[�X�̓r���h�ŃG���[�ɂȂ邩�珜�O����B
*/
void EscapeUnusableResPath(char *path)
{
	char *destPath = addExt(strx(path), "_git-escape");
//	char *destPath = changeExt(path, "_git-escape");
//	char *destPath = changeExt_cx(path, xcout("_git-escape.%s", getExt(path))); // �g���q�𐶂����Ă����K�v�͖���..

	cout("EURP\n");
	cout("< %s\n", path);
	cout("> %s\n", destPath);

	errorCase(!existPath(path));
	errorCase(existPath(destPath));

	movePath(path, destPath);

	memFree(destPath);
}
void PostGitMaskFile(char *file)
{
	char *maskedMarkFile = changeLocal(file, "====== MASKED ======");

	cout("[MASKED] %s\n", file);
	cout("-------> %s\n", maskedMarkFile);

	// �f�B���N�g��������Β��߂�B
	// ���ɍ쐬�ς݂̏ꍇ������B
	if (!accessible(maskedMarkFile))
		createFile(maskedMarkFile);

	memFree(maskedMarkFile);
}
void PostGitIgnoreFile(char *file)
{
	char *ignoredMarkFile = changeLocal(file, "====== IGNORED ======");

	cout("[IGNORED] %s\n", file);
	cout("--------> %s\n", ignoredMarkFile);

	// �f�B���N�g��������Β��߂�B
	// ���ɍ쐬�ς݂̏ꍇ������B
	if (!accessible(ignoredMarkFile))
		createFile(ignoredMarkFile);

	memFree(ignoredMarkFile);
}
