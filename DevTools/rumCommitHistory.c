/*
	rumCommitHistory.exe [/E COMMENT] [ROOT_DIR]

		/E       ... リビジョンを選択 (selectLines) してコメントに COMMENT を設定する。
		ROOT_DIR ... 省略した場合はカレントDIR

	----
	複数のリビジョンを削除する方法

	-- .rum_ディレクトリ群の親ディレクトリに移動して

	rumCommitHistory.exe /E 削除予定
	rumTrim
*/

#include "C:\Factory\Common\all.h"

static autoList_t *Revisions;

static void Search(char *rootDir)
{
	autoList_t *dirs = lsDirs(rootDir);
	char *dir;
	uint index;

	foreach (dirs, dir, index)
	{
		if (!_stricmp("rum", getExt(dir)))
		{
			char *revRootDir = combine(dir, "revisions");
			autoList_t *revDirs;
			char *revDir;
			uint revDir_index;

			revDirs = lsDirs(revRootDir);

			foreach (revDirs, revDir, revDir_index)
			{
				char *commentFile = combine(revDir, "comment.txt");
				char *comment;

				comment = readFirstLine(commentFile);

				addElement(Revisions, (uint)xcout("%s %s > %s", getLocal(revDir), dir, comment));

				memFree(comment);
				memFree(commentFile);
			}
			releaseDim(revDirs, 1);
			memFree(revRootDir);
		}
		else
		{
			Search(dir);
		}
	}
	releaseDim(dirs, 1);
}
static void Main2(char *rootDir)
{
	char *revision;
	uint index;

	Revisions = newList();
	Search(rootDir);
	rapidSortLines(Revisions);

	foreach (Revisions, revision, index)
		cout("%s\n", revision);
}
static void MultiSetComment(char *commentNew)
{
	autoList_t *selRevisions = selectLines(Revisions);
	char *line;
	uint index;

	errorCase(m_isEmpty(commentNew)); // 2bs

	// Confirm
	{
		cout("★★★★★ここから★★★★★\n");

		foreach (selRevisions, line, index)
			cout("%s\n", line);

		cout("★★★★★ここまで★★★★★\n");
		cout("これらのリビジョンのコメントを ...\n");
		cout("[%s]\n", commentNew);
		cout("... に設定します。\n");
		cout("続行？\n");

		if (clearGetKey() == 0x1b)
			termination(0);

		cout("続行します。\n");
	}

	foreach (selRevisions, line, index)
	{
		char *revision;
		char *rumDir;
		char *oldComment;
		char *commentFile;

		errorCase(!lineExp("<14,09> <1,,> > <1,,>", line)); // ? ! "YYYYMMDDhhmmss RUM-DIR > COMMENT"

		{
			char *p;
			char *q;

			p = line;
			revision = strxl(p, 14);
			p += 15;
			q = ne_strstr(p, " > ");
			rumDir = strxl(p, (uint)q - (uint)p);
			p = q + 3;
			oldComment = strx(p);
		}

		errorCase(m_isEmpty(rumDir));
		errorCase(m_isEmpty(oldComment));

		errorCase(!existDir(rumDir));

		commentFile = combine_xc(combine_xc(combine(rumDir, "revisions"), revision), "comment.txt");

		cout("! [%s]\n", oldComment);
		cout("< [%s]\n", commentNew);
		cout("> %s\n", commentFile);

		errorCase(!existFile(commentFile));

		writeOneLine(commentFile, commentNew);

		memFree(revision);
		memFree(rumDir);
		memFree(oldComment);
		memFree(commentFile);
	}
	releaseDim(selRevisions, 1);
}
int main(int argc, char **argv)
{
	char *comment = NULL;

	if (argIs("/E"))
	{
		comment = nextArg();
		errorCase(m_isEmpty(comment));
	}

	errorCase(hasArgs(2)); // ? 多過ぎる。-> オプション指定ミスか？

	if (hasArgs(1))
	{
		Main2(nextArg());
	}
	else
	{
		Main2(".");
	}

	if (comment)
	{
		MultiSetComment(comment);
	}
}
