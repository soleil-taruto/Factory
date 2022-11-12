#include "C:\Factory\Common\all.h"
#include "Define.h"

static void CharRecved(int chr)
{
	static int stock;

	if (stock)
	{
		char str[3];

		str[0] = stock;
		str[1] = chr;
		str[2] = '\0';

		line2JLine(str, 1, 0, 0, 0);

		cout("%s", str);

		stock = 0;
		return;
	}
	if (_ismbblead(chr))
	{
		stock = chr;
		return;
	}
	if (chr == '\n')
	{
		cout("\n");
		return;
	}
	m_toHalf(chr);
	cout("%c", chr);
}
static void BitRecved(uint bit)
{
	static uint chr;
	static uint bIndex;

	chr <<= 1;
	chr |= bit;

	bIndex++;

	if (bIndex == 8)
	{
		CharRecved((int)chr);
		chr = 0;
		bIndex = 0;
	}
}
static void DoRecv(void)
{
	uint evReady = eventOpen(EV_READY);
	uint evBit_0 = eventOpen(EV_BIT_0);
	uint evBit_1 = eventOpen(EV_BIT_1);
	autoList_t *evBits = newList();
	int death = 0;

	addElement(evBits, evBit_0);
	addElement(evBits, evBit_1);

	cout("+-----------------------------------+\n");
	cout("| エスケープキーを押すと中止します。|\n");
	cout("+-----------------------------------+\n");
	LOGPOS();

	collectEvents(evBit_0, 0); // clear
	collectEvents(evBit_1, 0); // clear
	eventSet(evReady);

	while (!death)
	{
		uint bit;

		if (mltHandleWaitForMillis(evBits, 2000, &bit))
		{
			BitRecved(bit);
			eventSet(evReady);
		}
		while (hasKey())
			if (getKey() == 0x1b)
				death = 1;
	}
	handleClose(evReady);
	handleClose(evBit_0);
	handleClose(evBit_1);
	releaseAutoList(evBits);

	LOGPOS();
}
int main(int argc, char **argv)
{
	DoRecv();
}
