#include "autoTable.h"

autoTable_t *newTable(uint (*createCell)(void), void (*releaseCell)(uint))
{
	autoTable_t *i = (autoTable_t *)memAlloc(sizeof(autoTable_t));

	errorCase(!createCell);
	errorCase(!releaseCell);

	i->Rows = newList();
	i->CreateCell = createCell;
	i->ReleaseCell = releaseCell;

	return i;
}
void releaseTable(autoTable_t *i)
{
	resizeTable(i, 0, 0);
	releaseAutoList(i->Rows);
	memFree(i);
}

void resizeTable(autoTable_t *i, uint w, uint h)
{
	uint rowidx;

	errorCase(w && !h); // ? (1 x 0), (2 x 0), (3 x 0)...

	while (h < getCount(i->Rows))
	{
		autoList_t *row = (autoList_t *)unaddElement(i->Rows);

		while (getCount(row))
		{
			i->ReleaseCell(unaddElement(row));
		}
		releaseAutoList(row);
	}
	while (getCount(i->Rows) < h)
	{
		addElement(i->Rows, (uint)newList());
	}
	for (rowidx = 0; rowidx < h; rowidx++)
	{
		autoList_t *row = getList(i->Rows, rowidx);

		while (w < getCount(row))
		{
			i->ReleaseCell(unaddElement(row));
		}
		while (getCount(row) < w)
		{
			addElement(row, i->CreateCell());
		}
	}
}
uint getTableWidth(autoTable_t *i)
{
	return getCount(i->Rows) ? getCount(getList(i->Rows, 0)) : 0;
}
uint getTableHeight(autoTable_t *i)
{
	return getCount(i->Rows);
}

uint *tableCellAt(autoTable_t *i, uint x, uint y)
{
	return directGetPoint(getList(i->Rows, y), x);
}
uint getTableCell(autoTable_t *i, uint x, uint y)
{
	return *tableCellAt(i, x, y);
}
void setTableCell(autoTable_t *i, uint x, uint y, uint e)
{
	*tableCellAt(i, x, y) = e;
}

uint *touchTableCellAt(autoTable_t *i, uint x, uint y)
{
	resizeTable(
		i,
		m_max(x + 1, getTableWidth(i)),
		m_max(y + 1, getTableHeight(i))
		);
	return tableCellAt(i, x, y);
}
uint refTableCell(autoTable_t *i, uint x, uint y)
{
	return *touchTableCellAt(i, x, y);
}
void putTableCell(autoTable_t *i, uint x, uint y, uint e)
{
	*touchTableCellAt(i, x, y) = e;
}

void resetTableCell(autoTable_t *i, uint x, uint y)
{
	uint *p = tableCellAt(i, x, y);

	i->ReleaseCell(*p);
	*p = i->CreateCell();
}
void resetTableRect(autoTable_t *i, uint l, uint t, uint w, uint h)
{
	uint x;
	uint y;

	for (x = 0; x < w; x++)
	for (y = 0; y < h; y++)
	{
		resetTableCell(i, l + x, t + y);
	}
}
void resetTable(autoTable_t *i)
{
	resetTableRect(i, 0, 0, getTableWidth(i), getTableHeight(i));
}

void swapTableCell(autoTable_t *i, uint x1, uint y1, uint x2, uint y2)
{
	uint swap_e = getTableCell(i, x1, y1);

	setTableCell(i, x1, y1, getTableCell(i, x2, y2));
	setTableCell(i, x2, y2, swap_e);
}
void twistTable(autoTable_t *i) // [0][0] - [max][max] ‚ðŽ²‚É”½“]
{
	uint x;
	uint y;

	if (getTableWidth(i) == getTableHeight(i))
	{
		for (x = 1; x < getTableWidth(i); x++)
		for (y = 0; y < x; y++)
		{
			swapTableCell(i, x, y, y, x);
		}
	}
	else
	{
		autoList_t *rowsNew = newList();

		for (x = 0; x < getTableWidth(i); x++)
		{
			autoList_t *rowNew = newList();

			for (y = 0; y < getTableHeight(i); y++)
			{
				addElement(rowNew, getTableCell(i, x, y));
			}
			addElement(rowsNew, (uint)rowNew);
		}
		releaseDim_BR(i->Rows, 2, NULL);
		i->Rows = rowsNew;
	}
}
void vTurnTable(autoTable_t *i) // [0][mid] - [max][mid] ‚ðŽ²‚É”½“]
{
	reverseElements(i->Rows);
}
void hTurnTable(autoTable_t *i) // [mid][0] - [mid][max] ‚ðŽ²‚É”½“]
{
#if 1
	uint rowidx;

	for (rowidx = 0; rowidx < getCount(i->Rows); rowidx++)
	{
		reverseElements(getList(i->Rows, rowidx));
	}
#else // SAME CODE
	twistTable(i);
	vTurnTable(i);
	twistTable(i);
#endif
}
void rot1Table(autoTable_t *i) // ŽžŒv‰ñ‚è‚É 90 “x‰ñ“]
{
	vTurnTable(i);
	twistTable(i);
}
void rot2Table(autoTable_t *i) // ŽžŒv‰ñ‚è‚É 180 “x‰ñ“]
{
	vTurnTable(i);
	hTurnTable(i);
}
void rot3Table(autoTable_t *i) // ŽžŒv‰ñ‚è‚É 270 “x‰ñ“]
{
	twistTable(i);
	vTurnTable(i);
}
