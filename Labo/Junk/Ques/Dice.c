/*
	1～8 個のサイコロを振って合計が大きい方が勝つときの勝率, 同数は自分の負け
	Y が自分の個数
	X が相手の個数
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\csv.h"
#include "C:\Factory\OpenSource\mt19937.h"

#define DICE_MAX 8
#define TEST_MAX 1000000

static uint PlayDice(uint diceNum)
{
	uint sum = 0;
	uint c;

	for (c = 0; c < diceNum; c++)
	{
		sum += mt19937_rnd(6) + 1;
	}
	return sum;
}
static double GetDiceWinRate(uint myDiceNum, uint osDiceNum)
{
	uint wincnt = 0;
	uint testcnt;

	cout("%u %u\n", myDiceNum, osDiceNum);

	for (testcnt = 0; testcnt < TEST_MAX; testcnt++)
	{
		uint mySum = PlayDice(myDiceNum);
		uint osSum = PlayDice(osDiceNum);
		int winflg;

		winflg = osSum < mySum ? 1 : 0;

//		cout("%u %u %u -> %u %u %d\n", myDiceNum, osDiceNum, testcnt, mySum, osSum, winflg);

		if (winflg)
			wincnt++;
	}
	return (double)wincnt / testcnt;
}
int main(int argc, char **argv)
{
	autoList_t *csv = newList();
	autoList_t *row;
	uint rowidx;
	uint colidx;

	mt19937_init();

	row = newList();
	addElement(row, (uint)strx("自分の個数＼相手の個数"));

	for (colidx = 0; colidx < DICE_MAX; colidx++)
		addElement(row, (uint)xcout("%u", colidx + 1));

	addElement(csv, (uint)row);

	for (rowidx = 0; rowidx < DICE_MAX; rowidx++)
	{
		row = newList();
		addElement(row, (uint)xcout("%u", rowidx + 1));

		for (colidx = 0; colidx < DICE_MAX; colidx++)
		{
			addElement(row, (uint)xcout("%f", GetDiceWinRate(rowidx + 1, colidx + 1)));
		}
		addElement(csv, (uint)row);
	}

	writeCSVFile_cx(c_getOutFile("a.csv"), csv);
	openOutDir();
}
