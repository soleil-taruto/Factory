#include "C:\Factory\Common\all.h"

static void Sort0326(autoList_t *lines)
{
	uint start;

	for (start = 0; start + 4 <= getCount(lines); start += 4)
	{
		autoList_t *entries = newList();
		uint index;

		for (index = 0; index < 4; index++)
		{
			addElement(entries, getElement(lines, start + index));
		}
		setElement(lines, start + 1, getElement(entries, 0));
		setElement(lines, start + 3, getElement(entries, 1));
		setElement(lines, start + 0, getElement(entries, 2));
		setElement(lines, start + 2, getElement(entries, 3));
	}
}
int main(int argc, char **argv)
{
	autoList_t *lines = newList();

	for (; ; )
	{
		lines = editTextLines_x(lines);

		if (getCount(lines) == 0)
		{
			break;
		}
		Sort0326(lines);
	}
	releaseDim(lines, 1);
}
