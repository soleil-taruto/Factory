/*
	評価値
		0 ... 評価不能 (局面が存在しない)
		1 ... 負け
		100000000 ... 未決着・タブー
		200000000 ... 未決着
		300000000 ... 引き分け
		400000000 ... 勝ち
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\rbTree.h"

#define FIELD_W_MAX 10
#define FIELD_H_MAX 10

// ---- input ----

static uint MaxDepth = 9;
static uint Field[FIELD_W_MAX][FIELD_H_MAX]; // [左 -> 右][下 -> 上]: (0-2: 空き, 自分, 相手)
static uint Field_W = 7;
static uint Field_H = 5;

// ----

static uint Depth; // 0: 自分の手番, 1: 相手の手番, 2: 自分, 3: 相手, ...

// ---- GetCurrEval() ----

#define NARABI_RANGE 4

static int IsNaranderu(int x, int y, int xAdd, int yAdd, uint kind, uint num)
{
	uint count = 0;
	uint i;

	for (i = 0; i < NARABI_RANGE; i++)
	{
		uint cell = Field[x][y];

		if (cell == kind)
		{
			count++;
		}
		else if (cell) // ? 別の石がある。
		{
			return 0; // もう並べないので、却下
		}
		x += xAdd;
		y += yAdd;
	}
	return count == num;
}
static uint GetNarabiCount(uint kind, uint num)
{
	uint count = 0;
	uint x;
	uint y;

	for (x = 0; x < Field_W; x++)
	for (y = 0; y < Field_H; y++)
	{
		if (x + NARABI_RANGE <= Field_W) // 右方向
		{
			if (IsNaranderu(x, y, 1, 0, kind, num))
				count++;
		}
		if (y + NARABI_RANGE <= Field_H) // 下方向
		{
			if (IsNaranderu(x, y, 0, 1, kind, num))
				count++;
		}
		if (x + NARABI_RANGE <= Field_W && y + NARABI_RANGE <= Field_H) // 右下方向
		{
			if (IsNaranderu(x, y, 1, 1, kind, num))
				count++;
		}
		if (x + NARABI_RANGE <= Field_W && NARABI_RANGE - 1 <= y) // 右上方向
		{
			if (IsNaranderu(x, y, 1, -1, kind, num))
				count++;
		}
	}
	return count;
}
static uint GetWinner(void) // ret: (0-2: 無し, 自分, 相手)
{
	if (GetNarabiCount(1, 4))
		return 1;

	if (GetNarabiCount(2, 4))
		return 2;

	return 0;
}
static int IsTaboo(void)
{
	uint x;
	uint y;

	for (x = 0; x + 5 <= Field_W; x++)
	for (y = 0; y + 1 <= Field_H; y++)
	{
		if (
			Field[x + 0][y + 0] == 0 &&
			Field[x + 1][y + 0] == 2 &&
			Field[x + 2][y + 0] == 2 &&
			Field[x + 3][y + 0] == 2 &&
			Field[x + 4][y + 0] == 0
			)
			return 1;
	}
	// old
	/*
	for (x = 0; x + 5 <= Field_W; x++)
	for (y = 0; y + 2 <= Field_H; y++)
	{
		if (
			// 上段
			Field[x + 1][y + 1] == 2 &&
			Field[x + 2][y + 1] == 2 &&
			Field[x + 3][y + 1] == 2 &&
			// 下段
			Field[x + 0][y + 0] == 0 &&
			Field[x + 4][y + 0] == 0
			)
			return 1;
	}
	*/
	return 0;
}
/*
	現局面のみ Field... を評価する。
*/
static uint GetCurrEval(void)
{
	uint winner = GetWinner();
	uint eval;

	if (winner == 1)
		return 400000000;

	if (winner == 2)
		return 1;

	eval = 200000000;
	eval += GetNarabiCount(1, 3) * 1000000;
	eval += GetNarabiCount(1, 2) * 10000;
	eval += 9900 - GetNarabiCount(2, 3) * 100;
	eval += 99   - GetNarabiCount(2, 3) * 1;

	if (IsTaboo())
		eval -= 100000000;

	return eval;
}

// ----

static int PutStone(uint x, uint kind) // ret: ? 石を置けた。
{
	uint y;

	for (y = 0; y < Field_H; y++)
	{
		if (!Field[x][y]) // ? 空き
		{
			Field[x][y] = kind;
			return 1;
		}
	}
	return 0;
}
static void EraseStone(uint x)
{
	uint y;

	for (y = Field_H - 1; y; y--)
		if (Field[x][y]) // ? ! 空き
			break;

	Field[x][y] = 0;
}

// ---- eval cache ----

static rbTree_t *EvalCache;

static void *GetFieldHash(void) // ret: c_
{
	static char stock[FIELD_W_MAX * FIELD_H_MAX + 1];
	char *p;
	uint x;
	uint y;

	p = stock;

	for (x = 0; x < Field_W; x++)
	for (y = 0; y < Field_H; y++)
	{
		*p = Field[x][y] + '0';
		p++;
	}
	*p = '\0';
	return stock;
}
static uint GetEvalCache(void *hash) // ret: 0 == キャッシュ無し
{
	if (!EvalCache)
		return 0;

	if (!rbtHasKey(EvalCache, (uint)hash))
		return 0;

	return rbtGetLastAccessValue(EvalCache);
}
static void SetEvalCache(void *hash, uint eval)
{
	if (!EvalCache)
		EvalCache = rbCreateTree((uint (*)(uint))strx, (sint (*)(uint, uint))strcmp, (void (*)(uint))memFree);

	rbtAddValue(EvalCache, (uint)hash, eval);
}

// ----

static uint GetEval(void);

/*
	現在の Depth, Field... の次の局面 (Depth + 1) を評価する。
*/
static void GetNextEvals(uint *dest) // 次手の評価リスト -> dest
{
	uint kind = Depth % 2 ? 2 : 1; // 現在の手番
	uint x;

	Depth++; // 次の手番にする。

	for (x = 0; x < Field_W; x++)
	{
		if (PutStone(x, kind))
		{
			dest[x] = GetEval();
			EraseStone(x); // 復帰
		}
		else
		{
			dest[x] = 0;
		}
	}
	Depth--; // 復帰
}
/*
	現在の Depth, Field... を評価する。
*/
static uint Real_GetEval(void) // ret: この局面の評価, 0 は返さない。
{
	uint evals[FIELD_W_MAX];
	uint ret;
	uint index;

	if (MaxDepth <= Depth) // ? 探索の上限に達した。-> これ以上先の手は読まない。
		return GetCurrEval();

	// 現局面で決着 -> これ以上先の手は読まない。
	{
		uint winner = GetWinner();

		if (winner == 1)
			return 400000000 + 100 - Depth; // 早い勝利を高く評価

		if (winner == 2)
			return 1;
	}

	GetNextEvals(evals);

	for (index = 0; index < Field_W; index++)
		if (evals[index])
			break;

	if (index == Field_W) // 次の手が無い -> 引き分け
	{
		ret = 300000000;
	}
	else if (Depth % 2) // 相手の手番 -> (自分にとって)最悪手を選ぶ
	{
		ret = UINTMAX;

		for (index = 0; index < Field_W; index++)
			if (evals[index])
				ret = m_min(ret, evals[index]);
	}
	else // 自分の手番 -> 最善手を選ぶ
	{
		ret = 0;

		for (index = 0; index < Field_W; index++)
			if (evals[index])
				ret = m_max(ret, evals[index]);
	}
	if (ret / 100000000 == 2) // ? 未決着コース
		if (IsTaboo())
			ret -= 100000000;

	return ret;
}
/*
	異なる手数でキャッシュを分けるべき！
	異なる手数で同じ局面は無いので、このままでいいや..
*/
static uint GetEval(void)
{
	uint ret = GetEvalCache(GetFieldHash());

	if (!ret)
	{
		ret = Real_GetEval();
		SetEvalCache(GetFieldHash(), ret);
	}
	return ret;
}

// ----

static void SetField(char *rawData) // rawData: 下 -> 上, 左 -> 右
{
	char *p = rawData;
	uint x;
	uint y;

	for (y = 0; y < Field_H; y++)
	for (x = 0; x < Field_W; x++)
	{
		switch (*p)
		{
		case '0':
		case '1':
		case '2':
			Field[x][y] = *p - '0';
			break;

		default:
			error();
		}
		p++;
	}
}
int main(int argc, char **argv)
{
readArgs:
	if (argIs("/D"))
	{
		MaxDepth = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/W"))
	{
		Field_W = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/H"))
	{
		Field_H = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/F"))
	{
		SetField(nextArg());
		goto readArgs;
	}

	errorCase(!m_isRange(MaxDepth, 0, IMAX));
	errorCase(!m_isRange(Field_W, 1, FIELD_W_MAX));
	errorCase(!m_isRange(Field_H, 1, FIELD_H_MAX));

readCmds:
	if (argIs("/C")) // curr eval
	{
		cout("%u\n", GetEval());
		goto readCmds;
	}
	if (argIs("/N")) // next eval
	{
		uint evals[FIELD_W_MAX];
		uint index;

		GetNextEvals(evals);

		for (index = 0; index < Field_W; index++)
			cout("%u\n", evals[index]);

		goto readCmds;
	}
}
