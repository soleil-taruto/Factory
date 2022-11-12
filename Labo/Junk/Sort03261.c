#include "C:\Factory\Common\all.h"

static void Sort0326(autoList_t *lines)
{
	uint start;

	for (start = 0; start + 18 <= getCount(lines); start += 18)
	{
		autoList_t *entries = newList();
		uint index;

		for (index = 0; index < 18; index++)
		{
			addElement(entries, getElement(lines, start + index));
		}
		setElement(lines, start +  0, getElement(entries,  0));
		setElement(lines, start +  2, getElement(entries,  1));
		setElement(lines, start +  3, getElement(entries,  2));
		setElement(lines, start +  4, getElement(entries,  3));
		setElement(lines, start +  5, getElement(entries,  4));
		setElement(lines, start +  6, getElement(entries,  5));
		setElement(lines, start +  7, getElement(entries,  6));
		setElement(lines, start + 11, getElement(entries,  7));
		setElement(lines, start +  1, getElement(entries,  8));
		setElement(lines, start +  9, getElement(entries,  9));
		setElement(lines, start + 10, getElement(entries, 10));
		setElement(lines, start +  8, getElement(entries, 11));
		setElement(lines, start + 12, getElement(entries, 12));
		setElement(lines, start + 14, getElement(entries, 13));
		setElement(lines, start + 17, getElement(entries, 14));
		setElement(lines, start + 13, getElement(entries, 15));
		setElement(lines, start + 15, getElement(entries, 16));
		setElement(lines, start + 16, getElement(entries, 17));
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
