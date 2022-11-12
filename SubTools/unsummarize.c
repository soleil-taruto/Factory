#include "C:\Factory\Common\all.h"

static int ChrEnumOpen  = '{';
static int ChrEnumClose = '}';
static int ChrEnumDlmtr = ',';

static char *OutBasePath = "C:\\Factory\\tmp\\unsummarize_";
static uint OutPathCount = 1;
static uint TmpPathCount = 1;

static autoList_t *RemFiles;
static autoList_t *CmpltFiles;

static char *GetOutPathCExt(uint count, char *ext)
{
	return toCreatablePath(xcout("%s%010u.%s", OutBasePath, count, ext), 1000);
}
static char *GetOutPath(void)
{
	return GetOutPathCExt(OutPathCount++, "txt");
}
static char *GetTmpPath(void)
{
	return GetOutPathCExt(TmpPathCount++, "tmp");
}

static int ProcOneRemFile(char *remFile, autoList_t *outFiles) // ret: ? 追加した。
{
	char *text = readText(remFile);
	char *eo;
	char *ez;
	char *p;
	char *token;
	autoList_t *tokens;
	uint index;
	uint enest = 0;
	int retval = 0;

	eo = mbs_strchr(text, ChrEnumOpen);

	if (!eo)
		goto endfunc;

	*eo = '\0';
	p = eo + 1;

	token = p;
	tokens = newList();

	for (; ; p = mbsNext(p))
	{
		errorCase(!*p);

		if (*p == ChrEnumDlmtr && enest == 0)
		{
			*p = '\0';
			addElement(tokens, (uint)strx(token));
			token = p + 1;
		}
		else if (*p == ChrEnumOpen)
		{
			enest++;
		}
		else if (*p == ChrEnumClose)
		{
			if (enest == 0)
			{
				break;
			}
			enest--;
		}
	}
	*p = '\0';
	addElement(tokens, (uint)strx(token));
	ez = p + 1;

	foreach (tokens, token, index)
	{
		char *outFile = GetTmpPath();
		FILE *fp;

		fp = fileOpen(outFile, "wt");

		writeToken(fp, text);
		writeToken(fp, token);
		writeToken(fp, ez);

		fileClose(fp);
		addElement(outFiles, (uint)outFile);
	}
	releaseDim(tokens, 1);
	retval = 1;

endfunc:
	memFree(text);
	return retval;
}
static void ProcRemFiles(void)
{
	uint index;

	for (index = 0; index < getCount(RemFiles); index++)
	{
		char *remFile = getLine(RemFiles, index);

		if (ProcOneRemFile(remFile, RemFiles))
		{
			removeFile(remFile);
		}
		else
		{
			addElement(CmpltFiles, (uint)strx(remFile));
		}
	}
}
static void StoreCmpltFiles(void)
{
	char *file;
	uint index;

	foreach (CmpltFiles, file, index)
	{
		char *outFile = GetOutPath();

		moveFile(file, outFile);
		memFree(outFile);
	}
}

int main(int argc, char **argv)
{
readArgs:
	if (argIs("/L")) // 単語リストの左括弧
	{
		ChrEnumOpen = nextArg()[0];
		goto readArgs;
	}
	if (argIs("/R")) // 単語リストの右括弧
	{
		ChrEnumClose = nextArg()[0];
		goto readArgs;
	}
	if (argIs("/D")) // 単語リストのデリミタ
	{
		ChrEnumDlmtr = nextArg()[0];
		goto readArgs;
	}
	if (argIs("/B")) // 出力ファイルのベース名
	{
		OutBasePath = nextArg();
		goto readArgs;
	}

	errorCase(!ChrEnumOpen);
	errorCase(!ChrEnumClose);
	errorCase(!ChrEnumDlmtr);
	errorCase(!OutBasePath);

	RemFiles = newList();
	CmpltFiles = newList();

	// add to RemFiles
	{
		autoList_t *files = newList();
		char *file;
		uint index;

		while (hasArgs(1))
		{
			char *path = nextArg();

			if (existDir(path))
			{
				autoList_t *subFiles = lsFiles(path);

				addElements(files, subFiles);
				releaseAutoList(subFiles);
			}
			else
			{
				addElement(files, (uint)strx(path));
			}
		}
		foreach (files, file, index)
		{
			char *remFile = GetTmpPath();

			copyFile(file, remFile);
			addElement(RemFiles, (uint)remFile);
		}
		releaseDim(files, 1);
	}

	ProcRemFiles();
	StoreCmpltFiles();

	releaseDim(RemFiles, 1);
	releaseDim(CmpltFiles, 1);
}
