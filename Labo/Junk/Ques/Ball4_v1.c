#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\rbTree.h"
#include "C:\Factory\OpenSource\sha512.h"

static uint WinCount = 4; // 1-IMAX
static uint TesuuMax = IMAX; // 1-IMAX
static uint NextPlayer; // 1, 2
static autoList_t *Table; // { [0=Empty, 1, 2] ... } ...
static uint XSize;
static uint YSize;

// ---- ResultCache ----

/*
	Table -> sha512
*/
static void *RC_GetKey(void) // ret: c_
{
	sha512_t *ctx = sha512_create();
	uint rowidx;
	autoList_t *row;
	uint colidx;
	uint value;
	uchar tmp[1];
	autoBlock_t gab;

	gndBlockVar(tmp, 1, gab);

	foreach (Table, row, rowidx)
	foreach (row, value, colidx)
	{
		tmp[0] = value;
		sha512_update(ctx, &gab);
	}
	sha512_makeHash(ctx);
	sha512_release(ctx);

	return sha512_hash;
}

#define RC_KEY_SIZE 16

static uint RC_DuplKey(uint prm)
{
	return (uint)memClone((void *)prm, RC_KEY_SIZE);
}
static sint RC_CompKey(uint prm1, uint prm2)
{
	void *key1 = (void *)prm1;
	void *key2 = (void *)prm2;

	return memcmp(key1, key2, RC_KEY_SIZE);
}
static void RC_FreeKey(uint prm)
{
	memFree((void *)prm);
}

static rbTree_t *ResultCache;

static void RegResult(autoList_t *result)
{
	if (!ResultCache)
		ResultCache = rbCreateTree(RC_DuplKey, RC_CompKey, RC_FreeKey);

	rbtAddValue(ResultCache, (uint)RC_GetKey(), (uint)copyAutoList(result));
}
static autoList_t *GetRegResult(void) // ret: NULL == 未登録
{
	if (ResultCache)
		if (rbtHasKey(ResultCache, (uint)RC_GetKey()))
			return (autoList_t *)rbtGetLastAccessValue(ResultCache);

	return NULL;
}

// ----

// ====

static int GetBall(int x, int y) // ret: -1=外, 0=Empty, 1, 2
{
//LOGPOS();
	if (m_isRange(x, 0, (int)XSize - 1) && m_isRange(y, 0, (int)YSize - 1))
	{
		return getElement(getList(Table, y), x);
	}
	return -1;
}
static void SetBall(int x, int y, int ball)
{
//LOGPOS();
	setElement(getList(Table, y), x, ball);
}
static int TryPut(int x)
{
	int y;
//LOGPOS();

	for (y = YSize - 1; 0 <= y; y--)
	{
		if (GetBall(x, y) == 0)
		{
			SetBall(x, y, NextPlayer);
			return 1;
		}
	}
	return 0;
}
static void Unput(int x)
{
	int y;
//LOGPOS();

	for (y = 0; y < YSize; y++)
	{
		if (1 <= GetBall(x, y))
		{
			SetBall(x, y, 0);
			break;
		}
	}
}
static int IsGameSet_XY(int x, int y, int ax, int ay)
{
	int ball = GetBall(x, y);
	int count;
//LOGPOS();

	if (ball < 1)
		return 0;

	for (count = 1; count < WinCount; count++)
	{
		x += ax;
		y += ay;

		if (GetBall(x, y) != ball)
			return 0;
	}
	return 1;
}
static int IsGameSet(void)
{
	int x;
	int y;
//LOGPOS();

	for (x = 0; x < XSize; x++)
	for (y = 0; y < YSize; y++)
	{
		if (
			IsGameSet_XY(x, y, 1, 0) ||
			IsGameSet_XY(x, y, 1, 1) ||
			IsGameSet_XY(x, y, 0, 1) ||
			IsGameSet_XY(x, y, -1, 1)
			)
			return 1;
	}
	return 0;
}

// ====

static uint GetMax(autoList_t *list)
{
	uint valmax = 0;
	uint index;
	uint value;
//LOGPOS();

	foreach (list, value, index)
	{
		m_maxim(valmax, value);
	}
	return valmax;
}

/*
	ret: [0=Lose, 1=Draw/置けない, 2=Win] ...
*/
static autoList_t *Simmulate(void)
{
	autoList_t *result;
	uint colidx;

	// キャッシュ有り -> それを返す。
	{
		result = GetRegResult();

		if (result)
			return copyAutoList(result);
	}
	result = newList();

	for (colidx = 0; colidx < XSize; colidx++)
	{
		uint value = 1;

		if (TryPut(colidx))
		{
			if (IsGameSet())
			{
				value = 2;
			}
			else if (!TesuuMax) // 手数の上限
			{
				value = 1; // その先はドローと見なす。
			}
			else
			{
				autoList_t *after;

				NextPlayer = 3 - NextPlayer;
				TesuuMax--;
				after = Simmulate();
				TesuuMax++;
				NextPlayer = 3 - NextPlayer;

				value = GetMax(after);
				value = 2 - value;

				releaseAutoList(after);
			}
			Unput(colidx);
		}
		addElement(result, value);
	}
	RegResult(result); // キャッシュ登録
	return result;
}
static void AddRow(char *line)
{
	autoList_t *row = newList();
	char *p;

	for (p = line; *p; p++)
	{
		errorCase(!strchr("012", *p));

		addElement(row, *p - '0');
	}
	addElement(Table, (uint)row);
}
static void CheckTable(void)
{
	autoList_t *row;
	uint rowidx;

	XSize = getCount(getList(Table, 0));
	YSize = getCount(Table);

	errorCase(!XSize);
//	errorCase(!YSize);

	foreach (Table, row, rowidx)
	{
		errorCase(getCount(row) != XSize);
	}
}
int main(int argc, char **argv)
{
	Table = newList();

readArgs:
	if (argIs("/W"))
	{
		WinCount = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/T"))
	{
		TesuuMax = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/1"))
	{
		NextPlayer = 1;
		goto readArgs;
	}
	if (argIs("/2"))
	{
		NextPlayer = 2;
		goto readArgs;
	}
	if (hasArgs(1))
	{
		AddRow(nextArg());
		goto readArgs;
	}
//LOGPOS();
	errorCase(!m_isRange(WinCount, 1, IMAX));
	errorCase(!m_isRange(TesuuMax, 1, IMAX));
	errorCase(!m_isRange(NextPlayer, 1, 2));
	CheckTable();
//LOGPOS();

	{
		autoList_t *result = Simmulate();
		uint index;
		uint value;

		foreach (result, value, index)
		{
			cout("%u\n", value);
		}
		releaseAutoList(result);
	}
}
