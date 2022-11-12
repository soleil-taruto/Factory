#include "C:\Factory\Common\all.h"

static autoList_t *Edges;

static void MakeEdges(void)
{
	int chr;

	Edges = newList();

	for (chr = 0x100; chr <= 0xffff; chr++)
	{
		if (!isJChar(chr - 1) && isJChar(chr))
		{
			addElement(Edges, (uint)chr);
		}
		if (isJChar(chr) && !isJChar(chr + 1))
		{
			addElement(Edges, (uint)chr);
		}
	}
}
int main(int argc, char **argv)
{
	FILE *fp = fileOpen(c_getOutFile("IsJChar_Java.txt"), "wb");
	int chr;
	uint index;

	MakeEdges();

	for (index = 0; index < getCount(Edges); index += 2)
	{
		int bgn = (int)getElement(Edges, index);
		int end = (int)getElement(Edges, index + 1);

		writeLine_x(fp, xcout("this.add(0x%04x, 0x%04x);", bgn, end));
	}
	fileClose(fp);

	openOutDir();
}
