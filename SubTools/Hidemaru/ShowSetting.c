/*
	ShowSetting.exe 入力ファイル

		入力ファイル ... RegExpToText.exe の出力ファイル
*/

#include "C:\Factory\Common\all.h"

enum
{
	RTL_PATH,  // レジストリのパス
	RTL_TYPE,  // 型
	RTL_VALUE, // 値

	RTL_SIZE, // 上のセットの個数
};

static autoList_t *RTLines;

static char *GetValue(char *path)
{
	uint index;

	for (index = 0; index < getCount(RTLines); index += RTL_SIZE)
		if (!strcmp(path, getLine(RTLines, index + RTL_PATH)))
			return getLine(RTLines, index + RTL_VALUE);

	return NULL;
}
static void ShowColor(char *title, autoBlock_t *bVal, uint baseIdx)
{
	uint ext;
	char *sExt;

	cout("%s ", title);
	cout("赤:%3u ", getByte(bVal, baseIdx + 0));
	cout("緑:%3u ", getByte(bVal, baseIdx + 1));
	cout("青:%3u ", getByte(bVal, baseIdx + 2));

	ext = getByte(bVal, baseIdx + 3);

	switch (ext)
	{
	case 0x00: sExt = strx("不透明"); break;
	case 0x02: sExt = strx("透明"); break;

	default:
		sExt = xcout("不明な値(%03u)", ext);
		break;
	}
	cout("%s\n", sExt);

	memFree(sExt);
}
static void Main2(char *regTextFile)
{
	RTLines = readLines(regTextFile);

	LOGPOS_T();

	cout("\n");
	cout("---- デザイン ----\n");
	cout("\n");

	{
		char *value = GetValue("HKCU\\Software\\Hidemaruo\\Hidemaru\\Default\\ColorSet");

		if (!value)
		{
			cout("強調表示の設定がありません。\n");
		}
		else
		{
			autoBlock_t *bVal = ab_fromHexLine(value);
			uint r;

			ShowColor("強調表示1", bVal, 48);
			ShowColor("強調表示2", bVal, 84);
			ShowColor("強調表示3", bVal, 264);
			ShowColor("強調表示4", bVal, 276);

			releaseAutoBlock(bVal);
		}
	}

	cout("\n");
	cout("---- デザイン / 強調表示 ----\n");
	cout("\n");

	{
		char *value = GetValue("HKCU\\Software\\Hidemaruo\\Hidemaru\\Default\\HilightWord");

		if (!value)
		{
			cout("単語の設定がありません。\n");
		}
		else
		{
			autoBlock_t *bVal = ab_fromHexLine(value);
			uint r;

			for (r = 0; r < getSize(bVal); )
			{
				uint len = getByte(bVal, r++);
				uint hh;
				uint c;
				char *word = strx("");
				char *shh;

				hh = getByte(bVal, r++);

				for (c = 0; c < len; c++)
					word = addChar(word, getByte(bVal, r++));

				hh = ((hh << 2) & 0xff) | (hh >> 6); // ROT: << 2

				cout("大文字小文字区別:%c ", hh & 0x20 ? 'N' : 'Y');
				cout("単語の検索:%c ",       hh & 0x80 ? 'Y' : 'N');
				cout("正規表現:%c ",         hh & 0x40 ? 'Y' : 'N');

				hh &= 0x1f;

				switch (hh)
				{
				case 0x04: shh = strx("強調表示1"); break;
				case 0x05: shh = strx("強調表示2"); break;
				case 0x06: shh = strx("強調表示3"); break;
				case 0x07: shh = strx("強調表示4"); break;

				default:
					shh = xcout("不明な表示方法(%02u)", hh); // HACK
					break;
				}
				cout("%s ", shh);
				cout("%s\n", word);

				memFree(word);
				memFree(shh);
			}
			releaseAutoBlock(bVal);
		}
	}

	cout("\n");
	cout("----\n");
	cout("\n");
}
int main(int argc, char **argv)
{
	Main2(nextArg());
}
