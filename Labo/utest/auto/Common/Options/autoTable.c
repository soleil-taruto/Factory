#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\autoTable.h"

static void Test01_3x3(void)
{
	autoTable_t *table = newTable(getZero, noop_u);

	// ----

	/*
		1	2	3
		4	5	6
		7	8	9
	*/

	resizeTable(table, 3, 3);

	setTableCell(table, 0, 0, 1);
	setTableCell(table, 1, 0, 2);
	setTableCell(table, 2, 0, 3);

	setTableCell(table, 0, 1, 4);
	setTableCell(table, 1, 1, 5);
	setTableCell(table, 2, 1, 6);

	setTableCell(table, 0, 2, 7);
	setTableCell(table, 1, 2, 8);
	setTableCell(table, 2, 2, 9);

	// ----

	twistTable(table);

	// ----

	/*
		1	4	7
		2	5	8
		3	6	9
	*/

	errorCase(getTableWidth(table)  != 3);
	errorCase(getTableHeight(table) != 3);

	errorCase(getTableCell(table, 0, 0) != 1);
	errorCase(getTableCell(table, 0, 1) != 2);
	errorCase(getTableCell(table, 0, 2) != 3);

	errorCase(getTableCell(table, 1, 0) != 4);
	errorCase(getTableCell(table, 1, 1) != 5);
	errorCase(getTableCell(table, 1, 2) != 6);

	errorCase(getTableCell(table, 2, 0) != 7);
	errorCase(getTableCell(table, 2, 1) != 8);
	errorCase(getTableCell(table, 2, 2) != 9);

	// ----

	twistTable(table);

	// ----

	/*
		1	2	3
		4	5	6
		7	8	9
	*/

	errorCase(getTableWidth(table)  != 3);
	errorCase(getTableHeight(table) != 3);

	errorCase(getTableCell(table, 0, 0) != 1);
	errorCase(getTableCell(table, 1, 0) != 2);
	errorCase(getTableCell(table, 2, 0) != 3);

	errorCase(getTableCell(table, 0, 1) != 4);
	errorCase(getTableCell(table, 1, 1) != 5);
	errorCase(getTableCell(table, 2, 1) != 6);

	errorCase(getTableCell(table, 0, 2) != 7);
	errorCase(getTableCell(table, 1, 2) != 8);
	errorCase(getTableCell(table, 2, 2) != 9);

	// ----

	releaseTable(table);
}
static void Test01_2x4(void)
{
	autoTable_t *table = newTable(getZero, noop_u);

	// ----

	/*
		1	5
		2	6
		3	7
		4	8
	*/

	resizeTable(table, 2, 4);

	setTableCell(table, 0, 0, 1);
	setTableCell(table, 0, 1, 2);
	setTableCell(table, 0, 2, 3);
	setTableCell(table, 0, 3, 4);

	setTableCell(table, 1, 0, 5);
	setTableCell(table, 1, 1, 6);
	setTableCell(table, 1, 2, 7);
	setTableCell(table, 1, 3, 8);

	// ----

	twistTable(table);

	// ----

	/*
		1	2	3	4
		5	6	7	8
	*/

	errorCase(getTableWidth(table)  != 4);
	errorCase(getTableHeight(table) != 2);

	errorCase(getTableCell(table, 0, 0) != 1);
	errorCase(getTableCell(table, 1, 0) != 2);
	errorCase(getTableCell(table, 2, 0) != 3);
	errorCase(getTableCell(table, 3, 0) != 4);

	errorCase(getTableCell(table, 0, 1) != 5);
	errorCase(getTableCell(table, 1, 1) != 6);
	errorCase(getTableCell(table, 2, 1) != 7);
	errorCase(getTableCell(table, 3, 1) != 8);

	// ----

	twistTable(table);

	// ----

	/*
		1	5
		2	6
		3	7
		4	8
	*/

	errorCase(getTableWidth(table)  != 2);
	errorCase(getTableHeight(table) != 4);

	errorCase(getTableCell(table, 0, 0) != 1);
	errorCase(getTableCell(table, 0, 1) != 2);
	errorCase(getTableCell(table, 0, 2) != 3);
	errorCase(getTableCell(table, 0, 3) != 4);

	errorCase(getTableCell(table, 1, 0) != 5);
	errorCase(getTableCell(table, 1, 1) != 6);
	errorCase(getTableCell(table, 1, 2) != 7);
	errorCase(getTableCell(table, 1, 3) != 8);

	// ----

	releaseTable(table);
}
static void Test01(void)
{
	Test01_3x3();
	Test01_2x4();

	cout("OK!\n");
}
int main(int argc, char **argv)
{
	Test01();
termination(0);
}
