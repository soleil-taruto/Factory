#include "ChangePETimeDateStamp.h"

void ChangePETimeDateStamp(char *file, uint t)
{
#if 1
	error_m("Modify PE Header is forbidden !!!"); // PEヘッダの書き換えは危険な香りがするので、禁止とする。@ 2022.5.1
#else
	FILE *fp = fileOpen(file, "r+b");
	uint peHedPos;
	uint optHedSize;

	LOGPOS();

	errorCase(readChar(fp) != 'M');
	errorCase(readChar(fp) != 'Z');

	fileSeek(fp, SEEK_SET, 0x3c);

	peHedPos = readValue(fp);

	fileSeek(fp, SEEK_SET, peHedPos);

	errorCase(readChar(fp) != 'P');
	errorCase(readChar(fp) != 'E');
	errorCase(readChar(fp) != 0x00);
	errorCase(readChar(fp) != 0x00);

	fileSeek(fp, SEEK_CUR, 0x04);

	writeValue(fp, t);

	fileSeek(fp, SEEK_CUR, 0x08);

	optHedSize = readValueWidth(fp, 2);

	cout("PE optional header size: %u\n", optHedSize);

errorCase(optHedSize != 224); // HACK: これ以外のサイズに出会ったら、CheckSum の位置に問題無いことを確認した上で、新しいサイズを許可する｜このチェックを削除する。

	if (0x44 <= optHedSize) // ? CheckSum までのサイズはある。
	{
		fileSeek(fp, SEEK_CUR, 0x02 + 0x40); // COFF header 残り + optional header の CheckSum まで

		writeValue(fp, 0x00000000); // CheckSum クリア
	}
	fileClose(fp);

	LOGPOS();
#endif
}
void ChangeAllPETimeDateStamp(char *dir, uint t)
{
	autoList_t *files = lssFiles(dir);
	char *file;
	uint index;

	foreach (files, file, index)
	{
		if (
			!_stricmp("EXE", getExt(file)) //||
//			!_stricmp("DLL", getExt(file))
			)
		{
			cout("file: %s\n", file);

			ChangePETimeDateStamp(file, t);
		}
	}
	releaseDim(files, 1);
}
uint GetPETimeDateStamp(char *file) // ret: 0 == 取得失敗
{
	FILE *fp = fileOpen(file, "rb");
	uint peHedPos;
	uint t = 0;

	LOGPOS();

	if (
		readChar(fp) != 'M' ||
		readChar(fp) != 'Z'
		)
		goto endFunc;

	fileSeek(fp, SEEK_SET, 0x3c);

	peHedPos = readValue(fp);

	fileSeek(fp, SEEK_SET, peHedPos);

	if (
		readChar(fp) != 'P' ||
		readChar(fp) != 'E' ||
		readChar(fp) != 0x00 ||
		readChar(fp) != 0x00
		)
		goto endFunc;

	fileSeek(fp, SEEK_CUR, 0x04);

	t = readValue(fp);

endFunc:
	fileClose(fp);
	return t;
}
