#include "all.h"

static int InputJTextMode;

/*
	シフトキーを押しながら何かキーを押したとき _kbhit() が 0 を返し続けることがある。
	キーバッファに追加される前で詰まっているように見える。
	シフトキーを数回連打すると解消する。
	シフトキーを押さずに何かキーを(1～数回)押下でも解消する。

	_kbhit() 互換性 Vista, 7 明記無し, XPでは再現せず -> Vista 以降で発生するのか?

	この現象を回避するには sock_hasKey() を使用すること。---> sock_hasKey() は押してないのに !0 を返すことがある。

	----
	2020.9.15

	_kbhit() の動作に問題がある。

	-- ソースここから (正常に動くプログラム)

	#include <stdio.h>

	main()
	{
		for (; ; )
			printf("%02x", _getch());
	}

	-- ソースここまで

	これを実行し標準入力から文字列を入力した場合の標準出力

	ABC      --->   414243
	いろは   --->   82a282eb82cd

	-- ソースここから (正常に動かないプログラム)

	#include <stdio.h>

	main()
	{
		for (; ; )
			while (_kbhit())
				printf("%02x", _getch());
	}

	-- ソースここまで

	これを実行し標準入力から文字列を入力した場合の標準出力

	ABC      --->   414243
	いろは   --->   a2a2a2a2a2 ...(省略)... a2a2a2a2a282a0a4a4a4a4a4 ...(省略, この辺でエンター押下)... a4a4a4a4a482a282a40d

	と、日本語を入力した場合明らかにおかしい。
	影響を受けるのは coInputLinePrn くらいだと思う。
	coInputLinePrn で日本語入力しなければならないような運用はしていないので、対応はせず、様子見とする。

	Visual C++ 2010 Express で発現する。Visual Studio Community 2019 では発現しない。
*/
static int TrueHasKey(void)
{
	/*
	static int called;

	if (!called)
	{
		cout("Caution: Used CRT _kbhit().\n");
		called = 1;
	}
	*/
	return _kbhit();
}
static int TrueGetKey(void)
{
	int key = _getch();

	if (!InputJTextMode && (key == 0x00 || key == 0xe0) && TrueHasKey())
	{
		key ^= 0xff; // to 0xff?? or 0x1f??
		key <<= 8;
		key |= _getch();
	}
	return key;
}

static autoList_t *BackKeyStack;

// ---- all proc pound key ----

#if 0 // 廃止 @ 2022.11.22

#define ALL_PROC_POUND_KEY_FILE "C:\\Factory\\tmp\\AllProcPoundKey.dat"

int ignoreAllProcPoundKey;

void setAllProcPoundKey(int key)
{
	errorCase(isFactoryDirDisabled());

	mutex();
	{
		if (key)
			writeOneValue(ALL_PROC_POUND_KEY_FILE, (uint)key);
		else
			removeFileIfExist(ALL_PROC_POUND_KEY_FILE);
	}
	unmutex();
}
static void CheckAllProcPoundKey(void)
{
	if (ignoreAllProcPoundKey)
		return;

	if (isFactoryDirDisabled())
		return;

	{
		static uint lastPassedTime;
		uint nowTime = now();

		if (!lastPassedTime)
			lastPassedTime = nowTime; // 最初の2秒間は通さない。

		if (nowTime < lastPassedTime + 2) // 2秒に1回通過
			return;

		lastPassedTime = nowTime;
	}

	mutex();
	{
		if (existFile(ALL_PROC_POUND_KEY_FILE) && !BackKeyStack)
		{
			int key = (int)readFirstValue(ALL_PROC_POUND_KEY_FILE);

			ungetKey(key);
		}
	}
	unmutex();
}

#else

int ignoreAllProcPoundKey;

void setAllProcPoundKey(int key)
{
	error_m("この機能は廃止されました。");
}
static void CheckAllProcPoundKey(void)
{
	// noop
}

#endif

// ----

int getKey(void)
{
	CheckAllProcPoundKey();

	if (BackKeyStack)
	{
		int key = (int)unaddElement(BackKeyStack);

		if (!getCount(BackKeyStack))
		{
			releaseAutoList(BackKeyStack);
			BackKeyStack = NULL;
		}
		return key;
	}
	return TrueGetKey();
}
void ungetKey(int key)
{
	if (!BackKeyStack)
		BackKeyStack = createAutoList(1);

	addElement(BackKeyStack, (uint)key);
}
int hasKey(void)
{
	CheckAllProcPoundKey();
	return BackKeyStack || TrueHasKey();
}
void clearKey(void)
{
	while (hasKey())
	{
		getKey();
	}
}
int clearGetKey(void)
{
	clearKey();
	return getKey();
}
int clearWaitKey(uint millis)
{
	clearKey();
	return waitKey(millis);
}
int clearCoWaitKey(uint millis)
{
	clearKey();
	return coWaitKey(millis);
}
int checkKey(int key)
{
	while (hasKey())
		if (getKey() == key)
			return 1;

	return 0;
}
int waitKey(uint millis)
{
	uint ms;

	for (; ; )
	{
		if (hasKey())
		{
			return getKey();
		}
		if (!millis)
		{
			break;
		}
		ms = m_min(millis, SLEEP_NICK_MILLIS);
		sleep(ms);
		millis -= ms;
	}
	return 0;
}
int coWaitKey(uint millis)
{
	uint elapse = 0;
	uint m;

	cout("%uミリ秒キー入力を待ちます...\n", millis);

	for (; ; )
	{
		if (hasKey())
		{
			uint key = getKey();

			cout("押されたキー=%02x\n", key);
			return key;
		}
		if (elapse == millis)
			break;

		m = m_min(millis - elapse, SLEEP_NICK_MILLIS);
		sleep(m);
		elapse += m;
		cout("\r%uミリ秒経ちました。", elapse);
	}
	cout("キー入力無し。\n");
	return 0;
}

autoList_t *editLines(autoList_t *lines) // ret: newList(), not NULL
{
	char *file = makeTempPath("txt");

	if (getCount(lines) == 1)
		writeOneLineNoRet(file, getLine(lines, 0));
	else
		writeLines(file, lines);

	execute(file); // ブロッキングで編集する。

	if (existFile(file))
	{
		lines = readLines(file);
		removeFile(file);
	}
	else
	{
		lines = newList();
	}
	memFree(file);
	return lines;
}
char *editLine(char *line) // ret: strx(), not NULL
{
	autoList_t lines;
	autoList_t *retLines;

	lines = gndOneElement((uint)line, (uint *)&retLines);
	retLines = editLines(&lines);
	line = untokenize(retLines, "");
	releaseDim(retLines, 1);
	return line;
}
autoList_t *editLines_x(autoList_t *lines)
{
	autoList_t *retLines = editLines(lines);

	releaseDim(lines, 1);
	return retLines;
}
char *editLine_x(char *line)
{
	char *retLine = editLine(line);

	memFree(line);
	return retLine;
}
autoList_t *inputLines(void)
{
	return editLines_x(newList());
}
char *inputLine(void)
{
	return editLine("");
}
autoBlock_t *inputTextAsBinary(void)
{
	char *file = makeTempPath("txt");
	autoBlock_t *block;

	editTextFile(file);
	createFileIfNotExist(file);
	block = readBinary(file);
	removeFile(file);
	memFree(file);
	return block;
}
void viewLines(autoList_t *lines)
{
	releaseDim(editLines(lines), 1);
}
void viewLine(char *line)
{
	memFree(editLine(line));
}
void viewLineNoRet_NB(char *line, int nonBlockingMode)
{
	char *file = makeTempPath("txt");
	FILE *fp;

	fp = fileOpen(file, "wt");
	writeToken(fp, line);
	fileClose(fp);

	if (!nonBlockingMode)
	{
		execute(file);
		removeFile(file);
	}
	else
	{
		execute_x(xcout("START \"\" \"%s\"", file));
	}
	memFree(file);
}
void viewLineNoRet(char *line)
{
	viewLineNoRet_NB(line, 0);
}

int coil_esc; // ? 最後の coInputLine() で ESCAPE が押された。

char *coInputLine(void)
{
	return coInputLinePrn(NULL);
}
char *coInputLinePrn(void (*printFunc)(char *jbuffer))
{
	char *buffer = strx("");
	char *jbuffer;
	int inputEnded = 0;

	coil_esc = 0;
	InputJTextMode = 1;

	for (; ; )
	{
		jbuffer = strx(buffer);
		line2JLine(jbuffer, 1, 0, 0, 1);

		if (printFunc)
			printFunc(jbuffer);
		else
			cout("\r%79s\r%s", "", jbuffer);

		if (inputEnded)
			break;

		memFree(jbuffer);
		ungetKey(getKey()); // 入力待ち。

		while (hasKey())
		{
			int chr = getKey();

			if (chr == 0x0d) // ENTER
			{
				inputEnded = 1;
				break;
			}
			if (chr == 0x1b) // ESCAPE
			{
				if (!buffer[0])
				{
					coil_esc = 1;
					inputEnded = 1;
					break;
				}
				buffer[0] = '\0';
			}
			else if (chr == 0x08) // BS
			{
				char *mp = buffer;
				char *p;

				for (p = buffer; *p; p = mbsNext(p))
				{
					mp = p;
				}
				*mp = '\0';
			}
			else
			{
				buffer = addChar(buffer, chr);
			}
		}
	}
	InputJTextMode = 0;
	cout("\n");
	memFree(buffer);
	return jbuffer;
}

#define WDROP_EXE_FILE "C:\\app\\WDrop\\WDrop.exe"

static char *DropPath_Win10(void)
{
	uint funcMtx;
	char *mtxName;
	uint mtxHdl;
	char *outFile;
	char *path;

	errorCase(isFactoryDirDisabled());
	errorCase(!existFile(WDROP_EXE_FILE)); // 外部コマンド存在確認

	funcMtx = mutexLock("{5396f16e-d695-4d3d-81a1-5b751d0d2068}");

	mtxName = xcout("{73d84a3e-26dc-49dd-9630-fd730e4d3303}_%u", getSelfProcessId());
	mtxHdl = mutexLock(mtxName);
	outFile = makeTempPath(NULL);

	cout("<D>");
//	cout("ドロップ先ウィンドウを開いています...\n");
//	cout("%s (%u)\n", getSelfFile(), getSelfProcessId());

	execute_x(xcout("START \"\" /B /WAIT %s \"%s\" %u \"%s\" \"%s\"", WDROP_EXE_FILE, getSelfFile(), getSelfProcessId(), mtxName, outFile));

//	cout("ドロップ先ウィンドウを閉じました。\n");

	mutexUnlock(mtxHdl);

	if (existFile(outFile))
	{
		path = readText_b(outFile);
		cout("%s</D>\n", path);
//		cout("<D>%s</D>\n", path);
		removeFile(outFile);

		// フルパスかどうか
		errorCase(!m_isalpha(path[0]) ||
			path[1] != ':' ||
			path[2] != '\\');

		errorCase(!existPath(path));
	}
	else
	{
		path =NULL;
		cout("</D>\n");
//		cout("<D></D>\n");
	}
	mutexUnlock(funcMtx);
	memFree(mtxName);
	memFree(outFile);
	clearKey();
	return path;
}
char *dropPath(void)
{
	char *path;

	if (isWindows10orLater())
		return DropPath_Win10();

	path = strx("");
	clearKey();
	cout("<D>");
	ungetKey(getKey());
	InputJTextMode = 1;

	while (hasKey())
	{
		path = addChar(path, getKey());
	}
	InputJTextMode = 0;
	trimEdge(path, '"');

	if (path[0] && !path[1]) // ? 打鍵した。
	{
		cout("</D>\n");
		goto cancelled;
	}
	cout("%s</D>\n", path);

	// フルパスかどうか
	if (!m_isalpha(path[0]) ||
		path[1] != ':' ||
		path[2] != '\\')
		error();

	if (!existPath(path))
		error();

	return path;

cancelled:
	memFree(path);
	return NULL;
}
char *dropFile(void) // ファイル以外 -> プログラム終了
{
	char *file = dropPath();

	if (!file || !existFile(file))
	{
		termination(0);
	}
	return file;
}
char *dropDir(void) // ディレクトリ以外 -> プログラム終了
{
	char *dir = dropPath();

	if (!dir || !existDir(dir))
	{
		termination(0);
	}
	return dir;
}
char *dropDirFile(void) // ディレクトリ・ファイル以外 -> プログラム終了
{
	char *path = dropPath();

	if (!path)
//	if (!path || !existDir(path) && !existDir(path))
	{
		termination(0);
	}
	return path;
}

// c_
char *c_dropPath(void)
{
	static char *stock;
	memFree(stock);
	return stock = dropPath();
}
char *c_dropFile(void)
{
	static char *stock;
	memFree(stock);
	return stock = dropFile();
}
char *c_dropDir(void)
{
	static char *stock;
	memFree(stock);
	return stock = dropDir();
}
char *c_dropDirFile(void)
{
	static char *stock;
	memFree(stock);
	return stock = dropDirFile();
}
