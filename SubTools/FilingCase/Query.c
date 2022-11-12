#include "C:\Factory\Common\all.h"
#include "libs\Query.h"

static void ExecuteQuery(char *query)
{
	autoList_t *rows = FC_ExecuteQuery(query);
	autoList_t *lines;
	char *line;
	uint index;

	if (!getCount(rows))
	{
		cout("no-rows\n");
		goto noData;
	}
	if (!getCount(getList(rows, 0)))
	{
		cout("no-cols\n");
		goto noData;
	}
	lines = newList();

	{
		autoList_t *row = newList();

		for (index = 0; index < getCount(getList(rows, 0)); index++)
			addElement(row, (uint)xcout("#%u", index + 1));

		insertElement(rows, 0, (uint)row);

		foreach (rows, row, index)
		{
			// 2bs ?
			{
				char *cell;
				uint cell_index;

				foreach (row, cell, cell_index)
				{
#if 1
					cell = FC_Retoken_x(cell);
					setElement(row, cell_index, (uint)cell);
#else // old
					line2JLine(cell, 1, 0, 0, 1);
#endif
				}
			}

			addElement(lines, (uint)untokenize(row, "\1"));
		}
	}

	shootingStarLines_CSP(lines, 0, '\1', ' ');

	{
		uint longest = 0;

		foreach (lines, line, index)
		{
			uint len = strlen(line);

			m_maxim(longest, len);
		}
		insertElement(lines, 1, (uint)repeatChar('-', longest));
	}

	foreach (lines, line, index)
		cout("%s\n", line);

	releaseDim(lines, 1);

noData:;
//	releaseDim(rows, 2); // dont!
}
int main(int argc, char **argv)
{
	if (hasArgs(1))
	{
		char *query = untokenize(getFollowArgs(0), " ");

		ExecuteQuery(query);

		memFree(query);
		return;
	}

	for (; ; )
	{
		char *query = coInputLine();

		if (!*query)
		{
			memFree(query);
			break;
		}
		ExecuteQuery(query);

		memFree(query);
	}
}
