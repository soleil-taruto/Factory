/*
	GitHisRemove.exe リポジトリ名

		実行中に選択 (selectLines) されたファイルを全履歴から削除する。

	GitHisRemove.exe /AUTO リポジトリ名

		エスケープされているリソースファイル (*._git-escape) について、
		エスケープされていなかった時代のファイルを全て削除する。

	GitHisRemove.exe /AUTOALL

		/AUTO を全てのリポジトリについて行う。

	----

	最新の状態に削除対象ファイルが存在すると、履歴から削除できないことがあるっぽい。
	当該ファイルを削除してコミット・pushしてから、履歴からの削除を行うべき。

	----> いちおう自動で行うようにした。
*/

#include "C:\Factory\Common\all.h"

#define REDIR_FILE "C:\\temp\\GitHisRemove_out.tmp"

#define EXT_GIT_ESCAPE "_git-escape"

static int IsSourceFile(char *file)
{
	return
		!_stricmp(getExt(file), "c") ||
		!_stricmp(getExt(file), "h") ||
		!_stricmp(getExt(file), "cs") ||
		!_stricmp(getExt(file), "cpp") ||
		!_stricmp(getExt(file), "bat") ||
		!_stricmp(getExt(file), "js");
}
static autoList_t *SelectAuto(autoList_t *files)
{
	autoList_t *dest = newList();
	char *file;
	uint index;

	foreach (files, file, index)
	{
		if (_stricmp(getExt(file), EXT_GIT_ESCAPE) && !IsSourceFile(file))
		{
			char *file1 = changeExt(file, EXT_GIT_ESCAPE);
			char *file2 = addExt(strx(file), EXT_GIT_ESCAPE);

			if (
				findJLineICase(files, file1) < getCount(files) ||
				findJLineICase(files, file2) < getCount(files)
				)
				addElement(dest, (uint)strx(file));

			memFree(file1);
			memFree(file2);
		}
	}
	releaseDim(files, 1);
	return dest;
}
static void Main2(char *repositoryName, int manualMode)
{
	char *repositoryDir = combine("C:\\huge\\GitHub", repositoryName);
	autoList_t *files = newList();
	char *file;
	uint index;

	errorCase_m(!existDir(repositoryDir), "そんなリポジトリありません。");

	addCwd(repositoryDir);
	{
		coExecute_x(xcout("C:\\temp\\bat\\go log --name-status > " REDIR_FILE));

		{
			autoList_t *lines = readLines(REDIR_FILE);
			char *line;
			uint index;

			foreach (lines, line, index)
			{
				if (strchr(line, '\t'))
				{
					autoList_t *tokens;
					uint token_index;

					errorCase(
						!lineExp("<1,,09AZaz>\t<1,, ~>", line) &&
						!lineExp("<1,,09AZaz>\t<1,, ~>\t<1,, ~>", line)
						);

					tokens = tokenize(line, '\t');

					for (token_index = 1; token_index < getCount(tokens); token_index++)
					{
						addElement(files, (uint)strx(getLine(tokens, token_index)));
					}
					releaseDim(tokens, 1);
				}
			}
			releaseDim(lines, 1);
		}

		files = autoDistinctLines(files);
		files = manualMode ? selectLines_x(files) : SelectAuto(files);

		foreach (files, file, index)
			cout("削除対象 ⇒ %s\n", file);

		if (manualMode)
		{
			cout("続行？\n");

			if (clearGetKey() == 0x1b)
				termination(0);

			cout("続行します。\n");
		}
		else
		{
			cout("続行します。(自動)\n");
		}

		foreach (files, file, index)
		{
			cout("削除 ⇒ %s\n", file);

			// 最新の状態に当該ファイルが存在すると削除できないことがあるっぽいので、削除する。
			{
				char *realFile = strx(file);

				restoreYen(realFile);

				if (!isFairRelPath(realFile, strlen_x(getCwd())))
				{
					cout("★★★相対パスとして認識できないため削除しません。\n"); // 日本語を含むパスなど有り得る。
				}
				else
				{
					if (existFile(realFile))
						coExecute_x(xcout("DEL \"%s\"", realFile));
				}
				memFree(realFile);
			}

			// 念のためコミット

			coExecute("C:\\temp\\bat\\go add *");
			coExecute("C:\\temp\\bat\\go commit -m \"remove from history\"");
			coExecute("C:\\temp\\bat\\go push");

			// ----

			coExecute_x(xcout("C:\\temp\\bat\\go filter-branch -f --tree-filter \"rm -f '%s'\" HEAD", file));
			coExecute("C:\\temp\\bat\\go push -f");

			LOGPOS();
		}
		LOGPOS();
	}
	unaddCwd();

	memFree(repositoryDir);
	releaseDim(files, 1);
}
int main(int argc, char **argv)
{
	if (argIs("/AUTOALL"))
	{
		autoList_t *dirs = lsDirs("C:\\huge\\GitHub");
		char *dir;
		uint index;

		foreach (dirs, dir, index)
		{
			Main2(getLocal(dir), 0);
		}
		releaseDim(dirs, 1);
		return;
	}
	if (argIs("/AUTO"))
	{
		Main2(nextArg(), 0);
		return;
	}
	Main2(nextArg(), 1);
}
