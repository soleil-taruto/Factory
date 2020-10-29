#pragma once

#include "C:\Factory\Common\all.h"

typedef struct autoTable_st // static member
{
	autoList_t *Rows;
	uint (*CreateCell)(void);
	void (*ReleaseCell)(uint);
}
autoTable_t;

autoTable_t *newTable(uint (*createCell)(void), void (*releaseCell)(uint));
void releaseTable(autoTable_t *i);

void resizeTable(autoTable_t *i, uint w, uint h);
uint getTableWidth(autoTable_t *i);
uint getTableHeight(autoTable_t *i);

uint *tableCellAt(autoTable_t *i, uint x, uint y);
uint getTableCell(autoTable_t *i, uint x, uint y);
void setTableCell(autoTable_t *i, uint x, uint y, uint e);

uint *touchTableCellAt(autoTable_t *i, uint x, uint y);
uint refTableCell(autoTable_t *i, uint x, uint y);
void putTableCell(autoTable_t *i, uint x, uint y, uint e);

void resetTableCell(autoTable_t *i, uint x, uint y);
void resetTableRect(autoTable_t *i, uint l, uint t, uint w, uint h);
void resetTable(autoTable_t *i);

void swapTableCell(autoTable_t *i, uint x1, uint y1, uint x2, uint y2);
void twistTable(autoTable_t *i);
void vTurnTable(autoTable_t *i);
void hTurnTable(autoTable_t *i);
void rot1Table(autoTable_t *i);
void rot2Table(autoTable_t *i);
void rot3Table(autoTable_t *i);
