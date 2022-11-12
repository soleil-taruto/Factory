/*
	isJChar(c1st << 8 | c2nd)
		CP932, 2バイト文字のみ判定(ASCII, 半角カナは判定できない)

	JIS0208.txt
		0x82A0 0x2422 0x3042 # HIRAGANA LETTER A
		SJIS          UTF16
		1st -> 2nd    big -> little

	"Aあ"
		SJIS    ->       41    82 A0
		UTF16LE -> ff fe 41 00 42 30
		UTF16BE -> fe ff 00 41 30 42

	JIS0208.txt
		SJIS  -> 昇順     重複ナシ
		JISX  -> 昇順     重複ナシ (予想)
		UTF16 -> ランダム 重複アリ

	CP932.txt
		CP932 -> 昇順     重複ナシ
		UTF16 -> ランダム 重複アリ

	対応する文字が複数あるときどの文字を返すか
		convCharUTF16ToSJIS()  -> 未定義
		convCharUTF16ToCP932() -> 文字コードの小さい方
*/

#include "all.h"

#define FILE_JIS0208 "C:\\Factory\\Resource\\JIS0208.txt"
#define FILE_CP932 "C:\\Factory\\Resource\\CP932.txt"

static uint RowCount;

static autoList_t *MakeMatrix(void)
{
	char *file = innerResPathFltr(FILE_JIS0208);
	FILE *fp;
	char *line;
	autoList_t *bufMtx = newList();

	fp = fileOpen(file, "rt");

	while (line = readLine(fp))
	{
		if (line[0] == '0')
		{
			errorCase(!lineExp("0x<4,09AF>\t0x<4,09AF>\t0x<4,09AF>\t#<>", line));

			addElement(bufMtx, toValueDigits_xc(strxl(line +  2, 4), hexadecimal));
			addElement(bufMtx, toValueDigits_xc(strxl(line +  9, 4), hexadecimal));
			addElement(bufMtx, toValueDigits_xc(strxl(line + 16, 4), hexadecimal));

			RowCount++;
		}
		memFree(line);
	}
	fileClose(fp);

	errorCase(!RowCount); // Anti-Empty

	return bufMtx;
}
static int (*GetMatrix(void))[3] // [S-JIS, X-JIS, UTF-16]
{
	static int (*mtx)[3];

	if (!mtx)
		mtx = (int (*)[3])unbindAutoList(MakeMatrix());

	return mtx;
}

static uint FindChar(uint cset, int mbchr)
{
	int (*mtx)[3] = GetMatrix();
	uint index;

	if (cset)
	{
		static autoList_t *subMtx[3];
errorCase(2 < cset);

		if (!subMtx[cset])
		{
			subMtx[cset] = newList();

			for (index = 0; index < RowCount; index++)
			{
				addElement(subMtx[cset], (uint)(mtx[index] + cset));
			}
			rapidSort(subMtx[cset], pSimpleComp);
		}
		index = binSearch(subMtx[cset], (uint)&mbchr, pSimpleComp);
errorCase(RowCount < index);

		if (index == getCount(subMtx[cset])) // ? not found
		{
			return 0; // どこでもいいから返す。
		}
		index = (getElement(subMtx[cset], index) - (uint)mtx) / sizeof(*mtx);
errorCase(RowCount <= index);
errorCase(mtx[index][cset] != mbchr);
	}
	else
	{
		uint idxmsk;

		index = 0;

		for (idxmsk = 0x1000; idxmsk; idxmsk >>= 1)
		{
			uint i = index | idxmsk;

			if (i < RowCount && mtx[i][cset] <= mbchr)
			{
				index = i;
			}
		}
	}
	return index;
}
static int IsExistChar(uint cset, int mbchr)
{
	return GetMatrix()[FindChar(cset, mbchr)][cset] == mbchr;
}
static int ConvChar(uint cset1, uint cset2, int mbchr)
{
	uint index = FindChar(cset1, mbchr);

	if (GetMatrix()[index][cset1] != mbchr)
	{
		return -1; // not found
	}
	return GetMatrix()[index][cset2];
}
static int ConvCharEC(uint cset1, uint cset2, int mbchr, int (*extraConv)(int))
{
	int ret = ConvChar(cset1, cset2, mbchr);

	if (ret == -1)
	{
		ret = extraConv(mbchr);
	}
	return ret;
}

int isSJISChar(int mbchr)
{
	return IsExistChar(0, mbchr);
}
int isNECSpecialChar(int mbchr)
{
	if (0x8740 <= mbchr && mbchr <= 0x879c) // NEC特殊文字の範囲
	{
		if (mbchr == 0x875e) // 未使用
		{
			// noop
		}
		else if (0x8776 <= mbchr && mbchr <= 0x877d) // 未使用
		{
			// noop
		}
		else if (mbchr == 0x877f) // 第２バイトの範囲外
		{
			// noop
		}
		else
		{
			return 1;
		}
	}
	return 0;
}
int isNECSelectIBMExtendChar(int mbchr)
{
	if (0xed40 <= mbchr && mbchr <= 0xeefc) // NEC選定IBM拡張文字の範囲
	{
		if (mbchr == 0xed7f) // 未使用
		{
			// noop
		}
		else if (0xedfd <= mbchr && mbchr <= 0xee3f) // 未使用 (広域)
		{
			// noop
		}
		else if (mbchr == 0xee7f) // 未使用
		{
			// noop
		}
		else if (0xeeed <= mbchr && mbchr <= 0xeeee) // 未使用
		{
			// noop
		}
		else
		{
			return 1;
		}
	}
	return 0;
}
int isIBMExtendChar(int mbchr)
{
	if (0xfa40 <= mbchr && mbchr <= 0xfc4b) // IBM拡張文字の範囲
	{
		if (mbchr == 0xfa7f) // 未使用
		{
			// noop
		}
		else if (0xfafd <= mbchr && mbchr <= 0xfb3f) // 未使用 (広域)
		{
			// noop
		}
		else if (mbchr == 0xfb7f) // 未使用
		{
			// noop
		}
		else if (0xfbfd <= mbchr && mbchr <= 0xfc3f) // 未使用
		{
			// noop
		}
		else
		{
			return 1;
		}
	}
	return 0;
}
int isJChar(int mbchr)
{
	return IsExistChar(0, mbchr) || isNECSpecialChar(mbchr) || isNECSelectIBMExtendChar(mbchr) || isIBMExtendChar(mbchr);
}
int isJCharP(char *p)
{
	return *p && p[1] && isJChar(*p << 8 | p[1]);
}
static int Conv932216(int mbchr, uint cset1, uint cset2)
{
	static autoList_t *dict[2];
	uint index;

	if (!dict[0]) // init
	{
		char *file = innerResPathFltr(FILE_CP932);
		autoList_t *lines;
		char *line;
		uint index;
		autoList_t *pairList = newList();
		uint *p;

		if (!existFile(file))
		{
			file = getLocal(file);
		}
		lines = readLines(file);

		foreach (lines, line, index)
		{
			if (line[0] == '0')
			{
				autoList_t *tokens;
				uint c;

				errorCase(
					!lineExp("0x<4,09AF>\t0x<4,09AF>\t#<>", line) &&
					!lineExp("0x<2,09AF>\t0x<4,09AF>\t#<>", line) &&
					!lineExp("0x<2,09AF>\t      \t#<>", line)
					);

				tokens = tokenize(line, '\t');

				if (getLine(tokens, 1)[0] != ' ') // 対応する UTF-16 が無い。-> CP932 側でも文字ではないはず。
				{
					for (c = 0; c < 2; c++)
					{
						addElement(pairList, toValueDigits(getLine(tokens, c) + 2, hexadecimal));
					}
				}
				releaseDim(tokens, 1);
			}
		}
		releaseDim(lines, 1);

		dict[0] = newList();
		dict[1] = newList();

		p = directGetList(pairList);

		for (index = 0; index < getCount(pairList); index += 2)
		{
			addElement(dict[0], (uint)(p + index + 0));
			addElement(dict[1], (uint)(p + index + 1));
		}
		rapidSort(dict[1], pSimpleComp2); // CP932 とアドレスは昇順 -> 同じ UTF-16 では CP932 の昇順

		foreach (dict[1], p, index)
		{
			if (index && *p == *(uint *)getElement(dict[1], index - 1))
			{
				desertElement(dict[1], index); // CP932 の大きい方を削除
				index--;
			}
		}
	}
	index = binSearch(dict[cset1], (uint)&mbchr, pSimpleComp);

	if (index == getCount(dict[cset1]))
	{
		return -1; // not found
	}
	return cset2[(int *)getElement(dict[cset1], index) - cset1];
}
int convCharCP932ToUTF16(int mbchr)
{
	return Conv932216(mbchr, 0, 1);
}
int convCharUTF16ToCP932(int mbchr)
{
	return Conv932216(mbchr, 1, 0);
}
int convCharSJISToUTF16(int mbchr)
{
	return ConvCharEC(0, 2, mbchr, convCharCP932ToUTF16);
}
int convCharUTF16ToSJIS(int mbchr)
{
	return ConvCharEC(2, 0, mbchr, convCharUTF16ToCP932);
}
