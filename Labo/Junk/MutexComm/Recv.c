#include "C:\Factory\Common\all.h"
#include "Common.h"

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
static void TryRecv(uint m0, uint m1, uint m2)
{
	M_Set(M_MUTEX_0 + m1, 1);

	// idling
	{
		static uint millis;

		if (!M_Get(M_MUTEX_0 + m2)) // ? 全てロック可能 == 送信中ではない。-> ウェイトを入れる。
		{
			if (millis < 200)
				millis++;

			sleep(millis);
		}
		else
		{
			millis = 0;
		}

		if (pulseSec(1, NULL))
			cmdTitle_x(xcout("Recv - %u", millis));
	}

	M_Set(M_MUTEX_0 + m0, 0);

	if (M_Get(M_BIT_0_0 + m1))
	{
		BitRecved(0);
	}
	if (M_Get(M_BIT_1_0 + m1))
	{
		BitRecved(1);
	}
}
static void DoRecv(void)
{
	int death = 0;

	cout("+-----------------------------------+\n");
	cout("| エスケープキーを押すと中止します。|\n");
	cout("+-----------------------------------+\n");
	LOGPOS();

	while (!death)
	{
		TryRecv(0, 1, 2);
		TryRecv(1, 2, 0);
		TryRecv(2, 0, 1);

		while (hasKey())
			if (getKey() == 0x1b)
				death = 1;
	}
	LOGPOS();
}
int main(int argc, char **argv)
{
	M_Init();
	DoRecv();
	M_Fnlz();

	cmdTitle("Recv");
}
