#include "C:\Factory\Common\all.h"

static void Test_addBytes(void)
{
	autoBlock_t *a = newBlock();
	autoBlock_t *b = newBlock();
	autoBlock_t gab;
	char *line;

	LOGPOS();

	addBytes(a, b);

	line = unbindBlock2Line(copyAutoBlock(a));
	errorCase(strcmp(line, ""));
	memFree(line);

	LOGPOS();

	addBytes(a, gndBlockLineVar("AAABBBCCCDDDEEE", gab));
	addBytes(b, a); // a:1 b:1
	addBytes(a, b); // a:2 b:1
	addBytes(b, a); // a:2 b:3
	addBytes(a, b); // a:5 b:3

	line = unbindBlock2Line(copyAutoBlock(a));
	errorCase(strcmp(line,
		"AAABBBCCCDDDEEE"
		"AAABBBCCCDDDEEE"
		"AAABBBCCCDDDEEE"
		"AAABBBCCCDDDEEE"
		"AAABBBCCCDDDEEE"
		));
	memFree(line);

	line = unbindBlock2Line(copyAutoBlock(b));
	errorCase(strcmp(line,
		"AAABBBCCCDDDEEE"
		"AAABBBCCCDDDEEE"
		"AAABBBCCCDDDEEE"
		));
	memFree(line);

	LOGPOS();

	releaseAutoBlock(a);
	releaseAutoBlock(b);
}
static void Test_unaddBytes(void)
{
	autoBlock_t *a = newBlock();
	autoBlock_t *b;
	autoBlock_t gab;
	char *line;
	uint count;

	LOGPOS();

	for (count = 10; count; count--)
		addBytes(a, gndBlockLineVar("AAABBBCCCDDDEEE", gab));

	line = unbindBlock2Line(copyAutoBlock(a));
	errorCase(strcmp(line,
		"AAABBBCCCDDDEEE" "AAABBBCCCDDDEEE" "AAABBBCCCDDDEEE"
		"AAABBBCCCDDDEEE" "AAABBBCCCDDDEEE" "AAABBBCCCDDDEEE"
		"AAABBBCCCDDDEEE" "AAABBBCCCDDDEEE" "AAABBBCCCDDDEEE"
		"AAABBBCCCDDDEEE"
		));
	memFree(line);

	b = unaddBytes(a, 66);

	line = unbindBlock2Line(copyAutoBlock(a));
	errorCase(strcmp(line,
		"AAABBBCCCDDDEEE" "AAABBBCCCDDDEEE" "AAABBBCCCDDDEEE"
		"AAABBBCCCDDDEEE" "AAABBBCCCDDDEEE" "AAABBBCCC"
		));
	memFree(line);

	line = unbindBlock2Line(copyAutoBlock(b));
	errorCase(strcmp(line,
		"DDDEEE"
		"AAABBBCCCDDDEEE" "AAABBBCCCDDDEEE" "AAABBBCCCDDDEEE"
		"AAABBBCCCDDDEEE"
		));
	memFree(line);

	LOGPOS();

	releaseAutoBlock(a);
	releaseAutoBlock(b);
}

int main(int argc, char **argv)
{
	Test_addBytes();
	Test_unaddBytes();
}
