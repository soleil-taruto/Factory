/*
	autoJavaDoc.exe [/S] [/F Javadocファイル] [/LSS | 対象ディレクトリ]
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\CRandom.h"

static int BatchMode;
static int IntoSubDir;
static autoList_t *JavaDoc;

static uint GetIndexFromPtn2(autoList_t *lines, uint index, char *ptn1, char *ptn2)
{
	char *ptn = xcout("%s%s", ptn1, ptn2);

	for (; index < getCount(lines); index++)
		if (!strcmp(ptn, getLine(lines, index)))
			break;

	memFree(ptn);
	return index;
}
static void DoAutoJavaDoc_File(char *file)
{
	autoList_t *lines = readLines(file);
	char *line;
	uint index;

	cout("%s\n", file);

	foreach (lines, line, index)
	{
		if (
			lineExp("<0,100,\t\t>public <>", line) ||
			lineExp("<0,100,\t\t>private <>", line) ||
			lineExp("<0,100,\t\t>protected <>", line)
			)
		{
			char *indentPtn = strx(line);
			uint indexDec = 0;

			*ne_strchr(indentPtn, 'p') = '\0';

			if (index && lineExp("<0,100,\t\t>@<>", getLine(lines, index - 1)))
			{
				index--;
				indexDec++;
			}
			// ? メソッド又はクラスがコメントアウトされている。
			if (index && lineExp("<0,100,\t\t>//*<>", getLine(lines, index - 1))) // エスケープ注意 / -> //
			{
				uint i = GetIndexFromPtn2(lines, index, indentPtn, "}"); // メソッド又はクラスの終端が見つかれば、そこまで進む。

				if (i < getCount(lines))
				{
					index = i;
					indexDec = 0;
				}
				goto endAddJavaDoc;
			}
			// ? 既にコメントがある。
			if (index && lineExp("<0,100,\t\t> *//", getLine(lines, index - 1))) // エスケープ注意 / -> //
			{
				goto endAddJavaDoc;
			}

			{
				char *javaDocLine;
				uint javaDocLineIndex;

				if (index && *getLine(lines, index - 1))
					insertElement(lines, index++, (uint)strx(""));

				foreach (JavaDoc, javaDocLine, javaDocLineIndex)
					insertElement(lines, index++, (uint)xcout("%s%s", indentPtn, javaDocLine));
			}

		endAddJavaDoc:
			index += indexDec;

			memFree(indentPtn);
		}
	}
	writeLines_cx(file, lines);
	cout("done\n");
}
static void Confirm(void)
{
	LOGPOS();

	if (BatchMode)
		return;

	cout("続行？\n");

	if (clearGetKey() == 0x1b)
		termination(0);

	cout("続行します。\n");
}
static void DoAutoJavaDoc_List(autoList_t *files)
{
	char *file;
	uint index;

	foreach (files, file, index)
		cout("file: %s\n", file);

	cout("files_count: %u\n", getCount(files));

	Confirm();

	foreach (files, file, index)
		DoAutoJavaDoc_File(file);

	releaseDim(files, 1);
}
static void DoAutoJavaDoc(char *dir)
{
	autoList_t *files = (IntoSubDir ? lssFiles : lsFiles)(dir);
	char *file;
	uint index;

	cout("dir: %s\n", dir);
	cout("IntoSubDir: %d\n", IntoSubDir);

	Confirm();

	foreach (files, file, index)
		if (!_stricmp("java", getExt(file)))
			DoAutoJavaDoc_File(file);

	releaseDim(files, 1);
}
int main(int argc, char **argv)
{
	char *javaDocFile = changeExt(getSelfFile(), "txt");

readArgs:
	if (argIs("/B"))
	{
		BatchMode = 1;
		goto readArgs;
	}
	if (argIs("/S"))
	{
		IntoSubDir = 1;
		goto readArgs;
	}
	if (argIs("/F"))
	{
		javaDocFile = nextArg();
		goto readArgs;
	}

	JavaDoc = readLines(javaDocFile);

	if (argIs("/LSS"))
	{
		DoAutoJavaDoc_List(readLines(FOUNDLISTFILE)); // g
		return;
	}
	if (hasArgs(1))
	{
		DoAutoJavaDoc(nextArg());
		return;
	}

	for (; ; )
	{
		DoAutoJavaDoc(c_dropDirFile());
		cout("\n");
	}
}
