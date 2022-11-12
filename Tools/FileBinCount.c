/*
	FileBinCount.exe (バイト列の16進数偶数桁 | /S 文字列) [入力ファイル]

	実行例
		FileBinCount.exe 0a 123.txt
			123.txt 内の改行の数を数える。

	0d == CR
	0a == LF
*/

#include "C:\Factory\Common\all.h"

static autoBlock_t *TargetPtn;

static void FileBinCount(char *file)
{
	FILE *fp = fileOpen(file, "rb");
	uint64 count = 0;

	for (; ; )
	{
		int chr = readChar(fp);

		if (chr == EOF)
			break;

		if (chr == b_(TargetPtn)[0])
		{
			uint index;

			for (index = 1; index < getSize(TargetPtn); index++)
			{
				chr = readChar(fp);

				if (chr == EOF)
					goto endLoop;

				if (chr != getByte(TargetPtn, index))
					break;
			}
			if (index < getSize(TargetPtn)) // ? コレジャナイ
			{
				fileSeek(fp, SEEK_CUR, -(sint64)(index - 1));
			}
			else // ? 見つかった
			{
				count++;
			}
		}
	}
endLoop:
	fileClose(fp);

	cout("%I64u\n", count);
}
int main(int argc, char **argv)
{
	if (argIs("/S"))
	{
		TargetPtn = ab_makeBlockLine(nextArg()); // "JKL" -> { 0x4a, 0x4b, 0x4c }
	}
	else
	{
		TargetPtn = makeBlockHexLine(nextArg()); // "4a4b4c" -> { 0x4a, 0x4b, 0x4c }
	}
	cout("PATTERN=%s\n", c_makeHexLine(TargetPtn));

	errorCase(!getSize(TargetPtn));

	if (hasArgs(1))
	{
		FileBinCount(nextArg());
	}
	else
	{
		for (; ; )
		{
			FileBinCount(c_dropFile());
			cout("\n");
		}
	}
}
