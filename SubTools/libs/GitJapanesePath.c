#include "GitJapanesePath.h"

#define RESTORE_BATCH "_run_me_for_restore_non_ascii_path_name.bat_"
//#define RESTORE_BATCH "_run_me_for_restore_japanese_path_name.bat_"

#define ESC_LOCALPATH "[Includes-Non-ASCII-characters]_0001"
//#define ESC_LOCALPATH "[Japanese-path-name]_0001"
//#define ESC_LOCALPATH "JP_0001"

static autoList_t *RBLines; // restore batch lines

static int HasJpn(char *str)
{
	char *p;

	for (p = str; *p; p++)
		if (_ismbblead(*p))
			return 1;

	return 0;
}
static char *PutDq(char *path)
{
	/*
		REN abc d=e     NG
		REN abc "d=e"   OK

- - -

m_isasciikana �ɂ��āA

       COPY abc d<����>e NG   COPY abc "d<����>e" NG
----------------------------------------------------
\x20   NG
&      NG
+      NG
,      NG
/      NG                     NG
:      NG ��1                 NG ��1
;      NG
<      NG                     NG
=      NG
>      NG                     NG
?      NG ��2                 NG ��2
\      NG ��1                 NG ��1
|      NG                     NG

��1 �h���C�u�E�f�B���N�g�����w�肵�����ƂɂȂ�B
��2 "?" �� "" �ɒu�������ăR�s�[�͐�������B

- - -

'%' �ɂ��āA

d%tmp%f �Ƃ����t�@�C���́i�G�N�X�v���[������j�쐬�\�����A
COPY abc d%tmp%f
COPY abc "d%tmp%f"
�͊��ϐ��ɒu���������� NG
COPY abc d%none%f
�̂悤�ɒ�`����Ă��Ȃ����ϐ��ł���� OK ???

execute (system) �֐�������s�����ꍇ�A�R�}���h�v�����v�g�ɒ��ړ��͂����ꍇ�Ɠ������ʁB

�o�b�`�t�@�C���̒����� % -> %% �ŃG�X�P�[�v���Ă��Ζ��Ȃ����ۂ��B

�ȉ��̃o�b�`�t�@�C�����쐬
DIR > abc
COPY abc d%%tmp%%f
�o�b�`�����s -> d%tmp%f ���쐬�����B

	*/
	if (
		strchr(path, ' ') ||
		strchr(path, '&') ||
		strchr(path, '+') ||
		strchr(path, ',') ||
		strchr(path, ';') ||
		strchr(path, '=')
		)
	{
		path = insertChar(path, 0, '"');
		path = addChar(path, '"');
	}
	path = replaceLine(path, "%", "%%", 0);

	return path;
}
static void SolveJpnPath(char *rootDir, char *realRootDir)
{
	autoList_t *paths = ls(rootDir);
	char *path;
	uint index;

	RemoveGitPaths(paths);

	sortJLinesICase(paths);

	foreach (paths, path, index)
	{
		char *localPath = getLocal(path);

		if (HasJpn(localPath))
		{
			char *dest = changeExt_xc(changeLocal(path, ESC_LOCALPATH), getExt(path));
			char *relDest;

			dest = toCreatablePath(dest, IMAX);
			relDest = changeRoot(strx(dest), realRootDir, NULL);

			movePath(path, dest);

			localPath = strx(localPath);
			localPath = PutDq(localPath);
			relDest = PutDq(relDest);

			addElement(RBLines, (uint)xcout("REN %s %s", relDest, localPath));

			memFree(localPath);
			memFree(dest);
			memFree(relDest);
		}
	}
	releaseDim(paths, 1);
	paths = lsDirs(rootDir);

	RemoveGitPaths(paths);

	sortJLinesICase(paths);

	foreach (paths, path, index)
	{
		SolveJpnPath(path, realRootDir);
	}
	releaseDim(paths, 1);
}
void SolveJapanesePath(char *rootDir)
{
	char *batFile = combine(rootDir, RESTORE_BATCH);

	LOGPOS();

	RBLines = newList();
	rootDir = makeFullPath(rootDir);

	SolveJpnPath(rootDir, rootDir);

	if (getCount(RBLines))
	{
		LOGPOS();
		reverseElements(RBLines);
		writeLines(batFile, RBLines);
	}
	releaseDim(RBLines, 1);
	memFree(rootDir);
	memFree(batFile);

	LOGPOS();
}
