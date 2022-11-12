#include "GitCommon.h"

static int IsGitPath(char *path)
{
	return (int)mbs_stristr(path, "\\.git"); // .git で始まるローカル名を含む
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
	マスクした結果、利用不可能になったリソースについて名前を退避する。
	読み込みエラーになるより、ファイルが無いエラーの方が利用不可能であることが分かりやすいだろうから。
	ソースはビルドでエラーになるから除外する。
*/
void EscapeUnusableResPath(char *path)
{
	char *destPath = addExt(strx(path), "_git-escape");
//	char *destPath = changeExt(path, "_git-escape");
//	char *destPath = changeExt_cx(path, xcout("_git-escape.%s", getExt(path))); // 拡張子を生かしておく必要は無い..

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

	// ディレクトリがあれば諦める。
	// 既に作成済みの場合もある。
	if (!accessible(maskedMarkFile))
		createFile(maskedMarkFile);

	memFree(maskedMarkFile);
}
void PostGitIgnoreFile(char *file)
{
	char *ignoredMarkFile = changeLocal(file, "====== IGNORED ======");

	cout("[IGNORED] %s\n", file);
	cout("--------> %s\n", ignoredMarkFile);

	// ディレクトリがあれば諦める。
	// 既に作成済みの場合もある。
	if (!accessible(ignoredMarkFile))
		createFile(ignoredMarkFile);

	memFree(ignoredMarkFile);
}
