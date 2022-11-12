#include "all.h"

static int IsAllZero(autoList_t *list)
{
	Price_t *i;
	uint index;

	foreach (list, i, index)
		if (i != NULL)
			return 0;

	return 1;
}

// ---- normalize ----

static void CompletionLead(autoList_t *list)
{
	uint index;
	uint rIndex;

	for (rIndex = 0; ; rIndex++)
		if (getElement(list, rIndex))
			break;

	for (index = 0; index < rIndex; index++)
		setElement(list, index, (uint)CopyPrice((Price_t *)getElement(list, rIndex)));
}
static void CompletionTrail(autoList_t *list)
{
	uint index;
	uint rIndex;

	for (rIndex = getCount(list) - 1; ; rIndex--)
		if (getElement(list, rIndex))
			break;

	for (index = getCount(list) - 1; rIndex < index; index--)
		setElement(list, index, (uint)CopyPrice((Price_t *)getElement(list, rIndex)));
}
static void CompletionMid(autoList_t *list)
{
	uint index;
	uint rIndex;

	for (rIndex = 1; rIndex + 1 < getCount(list); rIndex++)
	{
		if (!getElement(list, rIndex))
		{
			uint bgnidx = rIndex - 1;
			uint endidx;
			uint count;

			for (endidx = rIndex + 1; ; endidx++)
				if (getElement(list, endidx))
					break;

			count = endidx - bgnidx;

			for (index = 1; index < count; index++)
			{
				double bid =
					GetBid((Price_t *)getElement(list, bgnidx)) + (
					GetBid((Price_t *)getElement(list, endidx)) - GetBid((Price_t *)getElement(list, bgnidx))
					) * ((double)index /count);
				double ask =
					GetAsk((Price_t *)getElement(list, bgnidx)) + (
					GetAsk((Price_t *)getElement(list, endidx)) - GetAsk((Price_t *)getElement(list, bgnidx))
					) * ((double)index /count);

				setElement(list, bgnidx + index, (uint)CreatePrice(bid, ask));
			}
			rIndex = endidx;
		}
	}
}
static void Completion(autoList_t *list)
{
	uint index;

	errorCase(IsAllZero(list)); // ? ファイルはあるけど、中には１件も入ってない。
	// ダミーデータにしてもいいか..
	// 有り得ないはずなので、とりあえずエラー扱いにしておく。

	/*
		１回以上連続する失敗の直後に取得されたデータは何時のデータか分からないので捨てる。
		但し、その直後も失敗していたら捨てない。
	*/
	for (index = 0; index + 2 < getCount(list); index++)
	{
		Price_t *b = (Price_t *)getElement(list, index);
		Price_t *c = (Price_t *)getElement(list, index + 1);
		Price_t *d = (Price_t *)getElement(list, index + 2);

		if (!b && c && d)
		{
			ReleasePrice(c);
			setElement(list, index + 1, 0);
			index++;
		}
	}

	errorCase(IsAllZero(list)); // 2bs

	CompletionLead(list);
	CompletionTrail(list);
	CompletionMid(list);
}

// ----

static uint TimeToIndex(char *sTime)
{
	uint t;
	uint h;
	uint m;
	uint s;

	errorCase(!lineExp("<14,09>", sTime));

	t = toValue(sTime + 8);

	s = t % 100;
	t /= 100;
	m = t % 100;
	h = t / 100;

	errorCase(!m_isRange(h, 0, 23));
	errorCase(!m_isRange(m, 0, 59));
	errorCase(!m_isRange(s, 0, 59));
	errorCase(s & 1);

	t = h * 3600 + m * 60 + s;
	t /= 2;

	errorCase(!m_isRange(t, 0, 43200 - 1)); // 2bs

	return t;
}
static double ToDouble(char *str)
{
	double numer = 0.0;
	double denom = 1.0;
	int readDot = 0;
	char *p;

	for (p = str; *p; p++)
	{
		if (m_isdecimal(*p))
		{
			numer *= 10.0;
			numer += *p - '0';

			if (readDot)
				denom *= 10.0;
		}
		else if (*p == '.')
		{
			readDot = 1;
		}
	}
	return numer / denom;
}

autoList_t *LoadPriceDay(char *file) // ret: { Price_t *, ... }
{
	autoList_t *rows = readCSVFile(file);
	autoList_t *row;
	uint index;
	autoList_t *list = newList();

	setCount(list, 43200);

	foreach (rows, row, index)
	{
		char *sTime = getLine(row, 0);
		char *sBid  = getLine(row, 1);
		char *sAsk  = getLine(row, 2);
		uint index;
		double bid;
		double ask;

		index = TimeToIndex(sTime);
		bid = ToDouble(sBid);
		ask = ToDouble(sAsk);

		setElement(list, index, (uint)CreatePrice(bid, ask));
	}

	Completion(list);

	return list;
}
autoList_t *LoadDummyPriceDay(void) // ret: { Price_t *, ... }
{
	autoList_t *list = newList();
	uint index;

	for (index = 0; index < 43200; index++)
		addElement(list, (uint)CreatePrice(0.0, 0.0));

	return list;
}
