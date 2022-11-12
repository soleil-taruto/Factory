/*
	autoCSCommentPara.exe [変換を行うソースのディレクトリ]
*/

#include "C:\Factory\Common\all.h"

static void ApplyPara(autoList_t *lines, uint openTagLineIndex, uint closeTagLineIndex)
{
	int oneLineComment;
	uint index;

	errorCase(!lines);
	errorCase(closeTagLineIndex <= openTagLineIndex);
	errorCase(getCount(lines) <= closeTagLineIndex);

	oneLineComment = closeTagLineIndex - openTagLineIndex < 3;

	for (index = openTagLineIndex + 1; index < closeTagLineIndex; index++)
	{
		char *line = getLine(lines, index);

		// line は UTF-8 なので updateTagRng は使えない。-> updateAsciiTagRng を使う。

		if (updateAsciiTagRng(line, "<para>", "</para>"))
		{
			if (oneLineComment)
			{
				copyLine(lastTagRng.innerEnd, lastTagRng.end);
				copyLine(lastTagRng.bgn, lastTagRng.innerBgn);
			}
		}
		else
		{
			if (!oneLineComment)
			{
				char *p = strchr(line, '/');

				errorCase(!startsWith(p, "/// ")); // 2bs
				p += 4;

				line = insertLine(line, (uint)p - (uint)line, "<para>");
				line = addLine(line, "</para>");

				setElement(lines, index, (uint)line);
			}
		}
	}
}
static void AutoCSCommentPara(char *targetDir)
{
	autoList_t *files;
	char *file;
	uint index;

	targetDir = makeFullPath(targetDir);

	cout("T %s\n", targetDir);

	files = lssFiles(targetDir);

	foreach (files, file, index)
		if (_stricmp(getExt(file), "cs") || endsWithICase(file, ".Designer.cs") || !_stricmp(getLocal(file), "AssemblyInfo.cs")) // ? .cs ファイルではない || .Designer.cs ファイル || AssemblyInfo.cs ファイル
			*file = '\0';

	trimLines(files);
	sortJLinesICase(files);

	cout("C %u\n", getCount(files));

	foreach (files, file, index)
	{
		/*
		if (30 <= index)
		{
			cout("...\n");
			break;
		}
		*/
		cout("F %s\n", file);
	}

	{
		cout("続行？\n");

		if (clearGetKey() == 0x1b)
			termination(0);

		cout("続行します。\n");
	}

	LOGPOS();

	foreach (files, file, index)
	{
		autoList_t *lines = readLines(file);
		char *line;
		uint line_index;
		uint openTagLineIndex = UINTMAX;
		int modified = 0;

		cout("< %s\n", file);

		foreach (lines, line, line_index)
		{
			if (lineExp("<1,,\t\t>////// /<//<1,,>>", line)) // ? コメント・タグの終了
			{
				errorCase(openTagLineIndex == UINTMAX);
				LOGPOS();
				ApplyPara(lines, openTagLineIndex, line_index);
				LOGPOS();
				openTagLineIndex = UINTMAX;
				modified = 1;
			}
			else if (lineExp("<1,,\t\t>////// /<<1,,>>", line) && !lineExp("<>><>/<//<>", line)) // ? コメント・タグの開始 && この行でタグが完結していない。
			{
				errorCase(openTagLineIndex != UINTMAX);
				LOGPOS();
				openTagLineIndex = line_index;
			}
			else if (lineExp("<1,,\t\t>////// <1,,>", line)) // ? コメント・タグの途中
			{
				// noop
			}
			else // ? それら以外
			{
				errorCase(openTagLineIndex != UINTMAX);
			}
		}
		if (modified)
		{
			cout("> %s\n", file);
			writeLines(file, lines);
		}
		LOGPOS();
		releaseDim(lines, 1);
	}
	LOGPOS();
	releaseDim(files, 1);
	memFree(targetDir);
	LOGPOS();
}
int main(int argc, char **argv)
{
	if (hasArgs(1))
	{
		AutoCSCommentPara(nextArg());
		return;
	}

	{
		char *targetDir;

		cout("[C#]自動コメント(<para>付与・除去)を行うソースのディレクトリ：\n");
		targetDir = dropDir();

		AutoCSCommentPara(targetDir);

		memFree(targetDir);
	}
}
