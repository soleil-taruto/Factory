#include "all.h"

// ---- process properties ----

// ただの全てのソースから見える変数、ReadSysArgs() で設定可能な値とは限らない。

int sockServerMode;
int noErrorDlgMode;
uint sockConnectTimeoutSec = 20;
char *majorOutputLinePrefix = "";

// ----

int lastSystemRet; // ? ! コマンドが正常に実行できて 0 を返した。

void execute(char *commandLine)
{
	fflush(stdout);
	lastSystemRet = system(commandLine);
}
void execute_x(char *commandLine)
{
	execute(commandLine);
	memFree(commandLine);
}
void coExecute(char *commandLine)
{
	cout("cmdln: %s\n", commandLine);
	execute(commandLine);
}
void coExecute_x(char *commandLine)
{
	coExecute(commandLine);
	memFree(commandLine);
}
void cmdTitle(char *title)
{
	cmdTitle_x(strx(title));
}
void cmdTitle_x(char *title)
{
	unizChar(title, "%&<>^", '?');
	execute_x(xcout("TITLE %s", title));
	memFree(title);
}
static void ContextSwitching(void) // ts_
{
#if 0
	SwitchToThread(); // 確実にスイッチされるわけではない。基準がよく分からん。@ 2017.9.16
#else
	static uint count;

	count++; // どんな値になっても構わない。

	Sleep((count & 0x0f) / 0x0f);
#endif
}
void sleep(uint millis) // ts_
{
	if (millis)
		Sleep(millis);
	else
		ContextSwitching(); // sleep(0); のときは、単にコンテキストスイッチを行う。
}
void coSleep(uint millis)
{
	uint elapse = 0;

	cout("%uミリ秒待ちます...\n", millis);

	goto loopStart;

	while (elapse < millis)
	{
		uint m = m_min(millis - elapse, SLEEP_NICK_MILLIS);

		sleep(m);
		elapse += m;

	loopStart:
		cout("\r%uミリ秒経ちました。", elapse);
	}
	cout("\n");
}
void noop(void)
{
	// noop
}
void noop_u(uint dummy)
{
	// noop
}
void noop_uu(uint dummy1, uint dummy2)
{
	// noop
}
uint echo_u(uint prm)
{
	return prm;
}
uint getZero(void)
{
	return 0;
}
uint getZero_u(uint dummy)
{
	return 0;
}
uint getZero_uu(uint dummy1, uint dummy2)
{
	return 0;
}
char *getEnvLine(char *name) // ret: c_
{
	char *line;

	errorCase(!name);
	line = getenv(name);

	if (!line)
		line = "";

	return line;
}
#if 0 // for Test
static DWORD GetTickCount_TEST(void)
{
	static int initOnce;
	static uint baseTick;

	if (!initOnce)
	{
		initOnce = 1;
		baseTick = UINT_MAX - 10000 - GetTickCount();
	}
	return baseTick + GetTickCount();
}
#define GetTickCount() GetTickCount_TEST()
#endif
uint64 nowTick(void)
{
#if 1
	return GetTickCount64();
#else // old
	uint currTick = GetTickCount();
	static uint lastTick;
	static uint64 baseTick;
	uint64 retTick;
	static uint64 lastRetTick;

	if (currTick < lastTick) // ? カウンタが戻った -> オーバーフローした？
	{
		uint diffTick = lastTick - currTick;

//		LOGPOS(); // 再帰！！！

		if (UINTMAX / 2 < diffTick) // オーバーフローだろう。
		{
			baseTick += (uint64)UINT_MAX + 1;
		}
		else // オーバーフローか？
		{
			baseTick += diffTick; // 前回と同じ戻り値になるように調整する。
		}
	}
	lastTick = currTick;
	retTick = baseTick + currTick;
	errorCase(retTick < lastRetTick); // 2bs
	lastRetTick = retTick;
	return retTick;
#endif
}
uint now(void)
{
	return (uint)(nowTick() / 1000);
}
int pulseSec(uint span, uint *p_nextSec)
{
	static uint defNextSec;
	uint sec = now();

	if (!p_nextSec)
		p_nextSec = &defNextSec;

	if (sec < *p_nextSec)
		return 0;

	*p_nextSec = sec + span;
	return 1;
}
int eqIntPulseSec(uint span, uint *p_nextSec)
{
	static uint64 callPerCheck = 1; // uintだとカンストするかも。
	static uint64 count;

	count++;

	if (count % callPerCheck == 0 && pulseSec(span, p_nextSec))
	{
		count = m_max(callPerCheck / 3, count / 10);
		callPerCheck = m_max(1, count);
		count = 0;
		return 1;
	}
	return 0;
}
#if 0
uint getTick(void)
{
	return GetTickCount();
}
#endif
uint getUDiff(uint tick1, uint tick2) // ret: tick2 - tick1
{
	if (tick2 < tick1)
	{
		return 0xffffffffu - ((tick1 - tick2) - 1u);
	}
	return tick2 - tick1;
}
uint64 getUDiff64(uint64 tick1, uint64 tick2) // ret: tick2 - tick1
{
	if (tick2 < tick1)
	{
		return 0xffffffffffffffffui64 - ((tick1 - tick2) - 1ui64);
	}
	return tick2 - tick1;
}
sint getIDiff(uint tick1, uint tick2) // ret: tick2 - tick1
{
	uint diff = getUDiff(tick1, tick2);

	if (diff < 0x80000000u)
	{
		return (sint)diff;
	}
	return (-0x7fffffff - 1) + (sint)(diff - 0x80000000u);
}
sint64 getIDiff64(uint64 tick1, uint64 tick2) // ret: tick2 - tick1
{
	uint64 diff = getUDiff64(tick1, tick2);

	if (diff < 0x8000000000000000ui64)
	{
		return (sint64)diff;
	}
	return (-0x7fffffffffffffffI64 - 1) + (sint64)(diff - 0x8000000000000000ui64);
}
int isLittleEndian(void)
{
	uint dword = 1;
	return *(uchar *)&dword;
}

#define FILE_SHARE_COUNTER "C:\\Factory\\tmp\\Counter.txt"

uint64 nextCommonCount(void)
{
	char *line;
	uint64 counter;

	mutex();

	if (existFile(FILE_SHARE_COUNTER))
	{
		line = readFirstLine(FILE_SHARE_COUNTER);
		counter = toValue64(line);
		memFree(line);

		if (counter < 1000) // ? 小さすぎる。ファイルぶっ壊れた？
			goto initCounter;
	}
	else // カウンタ初期化
	{
	initCounter:
#if 1
		counter = (uint64)time(NULL) * 1000;
#else //old
		counter = toValue64Digits_xc(makeCompactStamp(NULL), hexadecimal) << 8;
		// ex. 1980/01/02 02:03:55 -> 0x1980010202035500
#endif
	}
	errorCase(counter == UINT64MAX); // カンスト

	if (UINT64MAX / 2 < counter)
		LOGPOS(); // カンスト注意

	line = xcout("%I64u", counter + 1);
	writeOneLine(FILE_SHARE_COUNTER, line);

	unmutex();

	memFree(line);
	return counter;
}
static char *c_GetTempSuffix(void)
{
	static char *ret;
	static int useFactoryTmpDir;

	if (ret)
		memFree(ret);
	else
		useFactoryTmpDir = isFactoryDirEnabled() && existDir("C:\\Factory\\tmp");

	if (!useFactoryTmpDir)
	{
		static uint pid;
		static uint64 pFATime;
		static uint64 counter;

		errorCase(counter == UINT64MAX); // カンスト

		if (UINT64MAX / 2 < counter)
			LOGPOS(); // カンスト注意

		if (!pid)
		{
			pid = getSelfProcessId();
			pFATime = (uint64)time(NULL);
		}
		ret = xcout("%x_%I64x_%I64x", pid, pFATime, counter);
		counter++;
	}
	else
	{
		ret = xcout("%I64u", nextCommonCount());
	}
	return ret;
}
char *makeTempPath(char *ext) // ext: NULL ok
{
	static char *pbase;
	char *path;

	if (!pbase)
	{
		int useFactoryTmpDir = isFactoryDirEnabled() && existDir("C:\\Factory\\tmp");

		if (!useFactoryTmpDir)
		{
			char *tmpDir = combine(getSelfDir(), "tmp");

			if (existDir(tmpDir))
				pbase = combine(getSelfDir(), "tmp\\");
			else
				pbase = combine(getSelfDir(), "$tmp$");

			memFree(tmpDir);
		}
		else
		{
			pbase = "C:\\Factory\\tmp\\";
		}
	}
	for (; ; )
	{
		path = xcout("%s%s", pbase, c_GetTempSuffix());

		if (ext)
			path = addExt(path, ext);

		if (!accessible(path))
			break;

		memFree(path);
	}
	return path;
}
char *makeTempFile(char *ext) // ext: NULL ok
{
	char *file = makeTempPath(ext);
	createFile(file);
	return file;
}
char *makeTempDir(char *ext) // ext: NULL ok
{
	char *dir = makeTempPath(ext);
	createDir(dir);
	return dir;
}
char *makeFreeDir(void)
{
	char *path;

	mutex();
	path = toCreatablePath(strx("C:\\1"), 999 - 1);
	createDir(path);
//	coExecute_x(xcout("ATTRIB.EXE +H %s", path));
	unmutex();

	return path;
}

#define SELFBUFFSIZE 1024
#define SELFBUFFMARGIN 16

char *getSelfFile(void)
{
	static char *fileBuff;

	if (!fileBuff)
	{
		fileBuff = memAlloc(SELFBUFFSIZE + SELFBUFFMARGIN);

		if (!GetModuleFileName(NULL, fileBuff, SELFBUFFSIZE))
			error();

		/*
			? フルパスの実行可能ファイルではない。
		*/
		errorCase(strlen(fileBuff) < 8); // 最短でも "C:\\a.exe"
		errorCase(!m_isalpha(fileBuff[0]));
		errorCase(memcmp(fileBuff + 1, ":\\", 2));
		errorCase(_stricmp(strchr(fileBuff, '\0') - 4, ".exe"));

		fileBuff = strr(fileBuff);
	}
	return fileBuff;
}
char *getSelfDir(void)
{
	static char *dirBuff;

	if (!dirBuff)
		dirBuff = getParent(getSelfFile());

	return dirBuff;
}
uint getSelfProcessId(void)
{
	static uint pid; // 0 は System Idle Process

	if (!pid)
	{
		pid = (uint)GetCurrentProcessId();
//		pid = (uint)_getpid(); // old
		errorCase(!pid);
	}
	return pid;
}

static char *GetOutDir(void)
{
	static char *dir;

	if (!dir)
		dir = makeTempDir("out");

	return dir;
}
char *getOutFile(char *localFile)
{
	return combine(GetOutDir(), localFile);
}
char *c_getOutFile(char *localFile)
{
	static char *outFile;
	memFree(outFile);
	return outFile = getOutFile(localFile);
}
char *c_getOutFile_x(char *localFile)
{
	static char *outFile;
	memFree(outFile);
	return outFile = getOutFile_x(localFile);
}
char *getOutFile_x(char *localFile)
{
	char *ret = getOutFile(localFile);
	memFree(localFile);
	return ret;
}
void openOutDir(void)
{
	execute_x(xcout("START %s", GetOutDir()));
}

// ---- args ----

autoList_t *tokenizeArgs(char *str)
{
	autoList_t *args = newList();
	autoBlock_t *buff = newBlock();
	char *p;
	int literalMode = 0;

	for (p = str; *p; p++)
	{
		if (literalMode)
		{
			if (*p == '"') // トークンの途中であってもリテラル終了
//			if (*p == '"' && (p[1] == ' ' || !p[1]))
			{
				literalMode = 0;
				goto addEnd;
			}
		}
		else
		{
			if (*p == ' ')
			{
				addElement(args, (uint)unbindBlock2Line(buff));
				buff = newBlock();
				goto addEnd;
			}
			if (*p == '"') // トークンの途中であってもリテラル開始
//			if (*p == '"' && !getSize(buff))
			{
				literalMode = 1;
				goto addEnd;
			}
		}

		if (isMbc(p))
		{
			addByte(buff, *p);
			p++;
		}
		else if (*p == '\\')
//		else if (*p == '\\' && (p[1] == '\\' || p[1] == '"'))
		{
			p++;
		}
		addByte(buff, *p);
	addEnd:;
	}
	addElement(args, (uint)unbindBlock2Line(buff));
	return args;
}

static autoList_t *Args;

static void ReadSysArgs(void)
{
	uint argi;

	for (argi = 0; argi < getCount(Args); )
	{
		char *arg = getLine(Args, argi);

		if (!_stricmp(arg, "//$")) // 読み込み中止
		{
			desertElement(Args, argi);
			break;
		}
		else if (!_stricmp(arg, "//-C"))
		{
			desertElement(Args, argi);
			coutOff = 1;
		}
		/*
			readResourceArgsText() と tokenizeArgs() で２回 "" の除去が行われる。
			-> !strcmp(getArg(x), "A B C") としたい場合、ファイルの当該行を ""A B C"" にする。
		*/
		else if (!_stricmp(arg, "//F")) // パラメータをファイルから読み込んで置き換える。
		{
			char *text;
			autoList_t *subArgs;

			desertElement(Args, argi);
			arg = (char *)desertElement(Args, argi);

			text = readResourceArgsText(innerResPathFltr(arg));
			subArgs = tokenizeArgs(text);
			memFree(text);

			while (getCount(subArgs))
				insertElement(Args, argi, (uint)unaddElement(subArgs));

			releaseAutoList(subArgs);
		}
		else if (!_stricmp(arg, "//R")) // パラメータをファイルから読み込んで置き換える。レスポンスファイル (改行を引数の区切りと見なす)
		{
			autoList_t *subArgs;

			desertElement(Args, argi);
			arg = (char *)desertElement(Args, argi);

			subArgs = readLines(arg);

			while (getCount(subArgs))
				insertElement(Args, argi, (uint)unaddElement(subArgs));

			releaseAutoList(subArgs);
		}
		/*
			プロセス終了時に閉じていないストリームは自動的に閉じてくれるけど、
			プロセス終了直後に他のプロセスが当該ファイルを開こうとすると、開けないことがある。
			-> ストリームを閉じるのが、シェルに制御を返すより遅れる？
			-> 何れにせよ明示的に閉じたほうが良い。
		*/
		else if (!_stricmp(arg, "//O")) // 標準出力(coutの出力)をファイルに書き出す。★注意：termination();しないとストリームを閉じない。
		{
			desertElement(Args, argi);
			arg = (char *)desertElement(Args, argi);

			setCoutWrFile(arg, "wt");
		}
		else if (!_stricmp(arg, "//A")) // 標準出力(coutの出力)をファイルに追記する。★注意：termination();しないとストリームを閉じない。
		{
			desertElement(Args, argi);
			arg = (char *)desertElement(Args, argi);

			setCoutWrFile(arg, "at");
		}
		else if (!_stricmp(arg, "//L")) // 標準出力(coutの出力)をログに出力する。★注意：termination();しないとストリームを閉じない。
		{
			desertElement(Args, argi);
			arg = (char *)desertElement(Args, argi);

			setCoutLogFile(arg); // arg == fileBase
		}
		else if (!_stricmp(arg, "//LA")) // 標準出力(coutの出力)をログに追記する。★注意：termination();しないとストリームを閉じない。
		{
			desertElement(Args, argi);
			arg = (char *)desertElement(Args, argi);

			setCoutLogFileAdd(arg); // arg == fileBase
		}
		else if (!_stricmp(arg, "//-E"))
		{
			desertElement(Args, argi);
			noErrorDlgMode = 1;
		}
		else if (!_stricmp(arg, "//CT"))
		{
			desertElement(Args, argi);
			arg = (char *)desertElement(Args, argi);

			sockConnectTimeoutSec = toValue(arg);

			cout("sockConnectTimeoutSec: %u\n", sockConnectTimeoutSec);

			errorCase(!m_isRange(sockConnectTimeoutSec, 1, 3600));
		}
		else if (!_stricmp(arg, "//MOLP"))
		{
			desertElement(Args, argi);
			arg = (char *)desertElement(Args, argi);

			majorOutputLinePrefix = arg;

			cout("majorOutputLinePrefix: %s\n", majorOutputLinePrefix);
		}
		else
		{
			argi++;
		}
	}
}
static autoList_t *GetArgs(void)
{
	if (!Args)
	{
		uint argi;

		Args = newList();

		for (argi = 1; argi < __argc; argi++)
		{
			addElement(Args, (uint)__argv[argi]);
		}
		ReadSysArgs();
	}
	return Args;
}

static uint ArgIndex;

int hasArgs(uint count)
{
	return count <= getCount(GetArgs()) - ArgIndex;
}
int argIs(char *spell)
{
	if (ArgIndex < getCount(GetArgs()))
	{
		if (!_stricmp(getLine(GetArgs(), ArgIndex), spell))
		{
			ArgIndex++;
			return 1;
		}
	}
	return 0;
}
char *getArg(uint index)
{
	errorCase(getCount(GetArgs()) - ArgIndex <= index);
	return getLine(GetArgs(), ArgIndex + index);
}
char *nextArg(void)
{
	char *arg = getArg(0);

	ArgIndex++;
	return arg;
}
char *nnNextArg(void)
{
	if (!hasArgs(1))
		return NULL;

	return nextArg();
}

void skipArg(uint count)
{
	for (; count; count--) nextArg();
}
/*
	ret: 残りのコマンド引数の index 番目以降全てを返す。
		index が残り個数と同じ場合 { } を返す。
*/
autoList_t *getFollowArgs(uint index)
{
	errorCase(getCount(GetArgs()) - ArgIndex < index);
	return recreateAutoList((uint *)directGetList(GetArgs()) + ArgIndex + index, getCount(GetArgs()) - ArgIndex - index);
}
autoList_t *allArgs(void)
{
	autoList_t *args = getFollowArgs(0);

	ArgIndex = getCount(GetArgs());
	return args;
}
uint getFollowArgCount(uint index)
{
	errorCase(getCount(GetArgs()) - ArgIndex < index);
	return getCount(GetArgs()) - (ArgIndex + index);
}
uint getArgIndex(void)
{
	return ArgIndex;
}
void setArgIndex(uint index)
{
	errorCase(getCount(GetArgs()) < index); // ? ! 全部読み終わった状態
	ArgIndex = index;
}

// ---- innerResPathFltr ----

// 主に C:\Factory の配下 との連携を想定する。関連 -> GetCollaboFile, GetCollaboDir

static char *FPP_Path;

static int FindPathParent(char *dir, char *localPath) // dir: abs_dir
{
	for (; ; )
	{
		FPP_Path = combine(dir, localPath);
//cout("FPP.1:%s\n", dir); // test
//cout("FPP.2:%s\n", FPP_Path); // test

		if (existPath(FPP_Path))
			return 1;

		memFree(FPP_Path);

		if (isAbsRootDir(dir))
			return 0;

		dir = getParent(dir);
	}
}
char *innerResPathFltr(char *path)
{
	if (isFactoryDirDisabled() && startsWithICase(path, "C:\\Factory\\")) // ? Factory 無効 && Factory 配下を参照
		goto go_search;

	if (getLocal(path) != path) // ? パスを指定している。
		if (existPath(path))
			goto foundPath;

go_search:
	if (FindPathParent(getSelfDir(), getLocal(path)))
	{
		path = FPP_Path;
		goto foundPath;
	}
	if (FindPathParent(getCwd(), getLocal(path)))
	{
		path = FPP_Path;
		goto foundPath;
	}
	cout("res_nf: %s\n", path);
writeOneLine(getOutFile("innerResPathFltr_path.txt"), path); // HACK
	error(); // not found

foundPath:
	cout("res: %s\n", path);
	return path; // path, strx() 混在しているが const char[] として扱うこと。
}

// ----

char *LOGPOS_Time(int mode)
{
	static char buff[93]; // 最長になるケース -> "307445734561825:51.615 18446744073709551615 +18446744073709551615 @ Thu Jan 01 16:59:59 3001" == 92文字
	static uint64 firstMillis;
	static uint64 lastMillis;
	uint64 millis = nowTick();

	if (!buff[0]) // ? 初回
	{
		sprintf(buff, "%I64u:%02u.%03u"
			,millis / 60000
			,(uint)((millis / 1000) % 60)
			,(uint)(millis % 1000)
			);

		firstMillis = millis;
	}
	else
	{
		sprintf(buff, "%I64u:%02u.%03u %I64u +%I64u"
			,millis / 60000
			,(uint)((millis / 1000) % 60)
			,(uint)(millis % 1000)
			,millis - firstMillis
			,millis - lastMillis
			);
	}
	if (mode == 'T')
	{
		char *p = strchr(buff, '\0');
		char *stamp = makeStamp(0L);

		*p++ = ' ';
		*p++ = '@';
		*p++ = ' ';
		strcpy(p, stamp);
		memFree(stamp);
	}
	lastMillis = millis;
	return buff;
}

// ----

void mkAppDataDir(void)
{
//	createDirIfNotExist("C:\\appdata"); // del @ 2022.1.23
	createDirIfNotExist("C:\\temp");
}

//#define APPDATA_ENVSFILE "C:\\appdata\\env" // del @ 2022.1.23
#define APPDATA_ENVSFILE "C:\\temp\\env"

char *getAppDataEnv(char *name, char *defval)
{
	static autoList_t *envs;
	char *env;
	uint index;
	uint nameLen;

	if (!envs)
	{
		LOGPOS();

		if (existFile(APPDATA_ENVSFILE))
			envs = readResourceLines(APPDATA_ENVSFILE);
		else
			envs = newList();
	}
	name = xcout("%s=", name);
	nameLen = strlen(name);

	foreach (envs, env, index)
		if (startsWith(env, name))
			break;

	memFree(name);

	if (env)
		return env + nameLen;

	return defval;
}
uint getAppDataEnv32(char *name, uint defval)
{
	char *sDefVal = xcout("%u", defval);
	uint value;

	value = toValue(getAppDataEnv(name, sDefVal));
	memFree(sDefVal);
	return value;
}

int isWindows10(void)
{
	uint ver[3];
	getWindowsVer(ver);
	return ver[0] == 10;
}
int isWindows10orLater(void)
{
	uint ver[3];
	getWindowsVer(ver);
	return ver[0] >= 10;
}
void getWindowsVer(uint ver[3])
{
	char *outFile = makeTempPath(NULL);
	char *line;
	char *sVer[3];

	execute_x(xcout("> \"%s\" ver", outFile));

	line = readText(outFile);

	toknext(line, "[");
	sVer[0] = toknext(NULL, ".");
	sVer[1] = toknext(NULL, ".");
	sVer[2] = toknext(NULL, "]");

	errorCase(!sVer[2]);

	ver[0] = toValue(sVer[0]);
	ver[1] = toValue(sVer[1]);
	ver[2] = toValue(sVer[2]);

	removeFile(outFile);

	memFree(line);
	memFree(outFile);
}
