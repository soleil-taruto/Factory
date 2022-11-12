#include "C:\Factory\Common\all.h"

static char *Separator = "/";

static autoList_t *NodeStk;
static autoList_t *DeepStk;

static void WrPath(FILE *wfp)
{
	char *node;
	uint index;

	foreach (NodeStk, node, index)
	{
		if (index)
			writeToken(wfp, Separator);

		writeToken(wfp, node);
	}
	writeChar(wfp, '\n');
}
static void Tree2PathList(char *rFile, char *wFile)
{
	FILE *rfp = fileOpen(rFile, "rt");
	FILE *wfp = fileOpen(wFile, "wt");

	NodeStk = newList();
	DeepStk = newList();

	for (; ; )
	{
		char *line = readLine(rfp);
		char *p;

		if (!line)
			break;

		for (p = line; *p && *p <= ' '; p++);

		if (*p)
		{
			char *node = strx(p);
			uint deep = (uint)p - (uint)line;

			while (getCount(DeepStk) && deep <= getLastElement(DeepStk))
			{
				memFree((char *)unaddElement(NodeStk));
				unaddElement(DeepStk);
			}
			addElement(NodeStk, (uint)node);
			addElement(DeepStk, deep);

			WrPath(wfp);
		}
		memFree(line);
	}
	fileClose(rfp);
	fileClose(wfp);

	releaseDim(NodeStk, 1);
	releaseAutoList(DeepStk);
}
int main(int argc, char **argv)
{
readArgs:
	if (argIs("/S"))
	{
		Separator = nextArg();
		goto readArgs;
	}

	Tree2PathList(hasArgs(1) ? nextArg() : c_dropFile(), getOutFile("PathList.txt"));
	openOutDir();
}
