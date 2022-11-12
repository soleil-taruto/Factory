#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	autoBlock_t *b = newBlock();
	uint s;

	fixBytes(b);

	for (
		s = 0;
		s <= 1000000;
		s += 1000
		)
		setSize(b, s);

	releaseAutoBlock(b);
}
