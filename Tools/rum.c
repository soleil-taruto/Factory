/*
	シンプル・バージョン管理プログラム

	rum.exe [/F] [/Q] [/-COLL] [/-NCHK] [対象ディレクトリ]

		/F ... バイナリ除外モード(Factoryモード) == .obj .exe C:\Factory\tmp を無視
		/Q ... 問い合わせを抑止する。
		/-COLL ... コリジョン検査を行わない。多少速くなるかも。   <---   廃止 @ 2020.10.29
		/-NCHK ... 入れ子チェックを行わない。                     <---   廃止 @ 2020.10.7

	rum.exe [/E | /T | /HA | /H | /1A | /1 | /R | /L] [/D] [.rum_ディレクトリ]

		/E  ... コメント編集
		/T  ... ごみファイル削除 + コメントが "削除予定" または "削除予定, " で始まるリビジョンを削除する。
		                           ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~   <---   廃止 @ 2021.3.16
		/HA ... ファイルの変更履歴を取得、ファイルの一覧を全リビジョンから取得
		/H  ... ファイルの変更履歴を取得、ファイルの一覧を最後のリビジョンから取得
		/1A ... 最後のファイルを取得、ファイルの一覧を全リビジョンから取得
		/1  ... 最後のファイルを取得、ファイルの一覧を最後のリビジョンから取得
		/R  ... 最後のリビジョンが修正無し且つコメント無しであれば削除する。
		/L  ... パスリストのみ出力する。
		/D  ... 常に C:\NNN にリストアする。(NNN は 1～999)

	rum.exe (/C [COMMENT] | /C-)

		自動コメント編集｜クリア

	---
	関連コマンド

	CheckRum

		.rum ディレクトリの破損をチェックする。

	MergeRum

		2つの .rum ディレクトリを何方か一方にマージする。

	rumCommitHistory

		カレントディレクトリ配下の全ての .rum ディレクトリについて、コミット履歴を表示する。

	rumSearch

		「指定された .rum ディレクトリ(複数可)」または「カレントディレクトリ配下の全ての .rum ディレクトリ」について grep する。

	rlss

		「指定されたディレクトリまたはカレントディレクトリ」配下の全ての .rum ディレクトリについてパスリストを表示する。

	----
	使い方

	"EagleJump" というディレクトリを保存(リポジトリみたいなものを作成)する。

	> rum EagleJump

	テキストエディタが起動するのでコメントを記入して保存する。
	続行？でエスケープキーを押すとキャンセルする。
	続行すると "EagleJump.rum" が生成される。
	2回目以降も同じコマンド、同じ手順。

	- - -

	復元するには以下のコマンドを実行する。

	> rum EagleJump.rum

	テキストエディタが起動するので復元するリビジョンの行を削除するか、復元するリビジョンの行「以外」を削除して保存する。
	復元する行を削除した場合は L 復元する行「以外」を削除した場合は R を押す。
	"EagleJump" が無い場合は "EagleJump" に、"EagleJump" が有る場合は C:\NNN にリストアする。(NNN は 1～999)

	C:\NNN は zz で一斉に削除できる。

	----
	注意

	属性とタイムスタンプは保存しない。
	同じMD5で内容が異なるファイルは登録できない。-> エラーになる。(リビジョンは作成されない。.rum は壊れないが、rum /t 必要)
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\OpenSource\md5.h"
#include "C:\Factory\SubTools\libs\ChangePETimeDateStamp.h"

#define EXT_STOREDIR "rum"

#define DIR_FILES "files"
#define DIR_REVISIONS "revisions"

#define FILE_REV_COMMENT "comment.txt"
#define FILE_REV_FILES "files.txt"
#define FILE_REV_TREE "tree.txt"

#define DEFAULT_COMMENT "コメント無し"

// 廃止 @ 2021.3.16
//#define DELETABLE_COMMENT_01 "削除予定"
//#define DELETABLE_COMMENT_02_START_PTN "削除予定, "

#define AUTO_COMMENT_FILE "C:\\temp\\rum-auto-comment.txt"
//#define AUTO_COMMENT_FILE "C:\\appdata\\rum-auto-comment.txt" // del @ 2022.1.23
//#define AUTO_COMMENT_FILE "C:\\Factory\\tmp\\auto-comment.txt" // qrumall /D で消される。

static uint ErrorLevel;

/*
	( ハッシュ値 + SP + パス ) のパスを比較
*/
static sint HashedPathComp(uint a, uint b)
{
	return !_stricmp((char *)a + 33, (char *)b + 33);
}

static char *GetAutoComment(void) // ret: NULL == 未登録
{
	char *comment = NULL;

	if (existFile(AUTO_COMMENT_FILE))
	{
		comment = readText(AUTO_COMMENT_FILE);
		line2JLine(comment, 1, 0, 0, 1);
		trim(comment, ' ');

		if (!*comment)
		{
			memFree(comment);
			comment = NULL;
		}
	}
	return comment;
}
static char *GetComment(char *storeDir, char *stamp)
{
	char *comment;

	addCwd(storeDir);
	addCwd(DIR_REVISIONS);
	addCwd(stamp);

	comment = readFirstLine(FILE_REV_COMMENT);
	errorCase(m_isEmpty(comment)); // 2bs

	unaddCwd();
	unaddCwd();
	unaddCwd();

	return comment;
}
static autoList_t *MakeRevisionList(char *storeDir)
{
	autoList_t *revisions;

	addCwd(storeDir);
	revisions = ls(DIR_REVISIONS);
	unaddCwd();

	eraseParents(revisions);
	rapidSortLines(revisions);

	// コメントの追加
	{
		char *stamp;
		uint index;

		foreach (revisions, stamp, index)
		{
			char *comment;

			addCwd(storeDir);
			addCwd(DIR_REVISIONS);
			addCwd(stamp);

			comment = readFirstLine(FILE_REV_COMMENT);
			line2JLine(comment, 1, 0, 0, 1);
			trim(comment, ' ');

			errorCase(!*comment); // 2bs

			stamp = addLine_x(stamp, xcout(" %s", comment));
			setElement(revisions, index, (uint)stamp);

			memFree(comment);

			unaddCwd();
			unaddCwd();
			unaddCwd();
		}
	}
	return revisions;
}
static autoList_t *SelectStamps(char *storeDir)
{
	autoList_t *revisions = MakeRevisionList(storeDir);
	autoList_t *stamps;
	char *stamp;
	uint index;
	char *p;

	stamps = selectLines(revisions);

	foreach (stamps, stamp, index)
	{
		p = strchr(stamp, '\x20');
		errorCase(!p);
		*p = '\0';
	}
	releaseDim(revisions, 1);
	return stamps;
}
static void PostRestoreFile(char *wFile)
{
	char *wExt = getExt(wFile);

	if (
		!_stricmp(wExt, "DLL") ||
		!_stricmp(wExt, "EXE") ||
		!_stricmp(wExt, "LIB") ||
		!_stricmp(wExt, "OBJ")
		)
	{
		uint t = GetPETimeDateStamp(wFile);

		if (t)
		{
			uint64 stamp = getFileStampByTime((time_t)t);

			cout("%I64u > %s\n", stamp, wFile);

			setFileStamp(wFile, 0ui64, 0ui64, stamp);
		}
	}
}

static void EditComment(char *storeDir)
{
	autoList_t *revisions = MakeRevisionList(storeDir);
	char *line;
	uint index;

	cout("+----------------+\n");
	cout("| コメントの編集 |\n");
	cout("+----------------+\n");
	sleep(500);

	revisions = editLines_x(revisions);

	for (; ; )
	{
		int key;

		cout("続行？[Enter/Esc]\n");

		key = clearGetKey();

		if (key == 0x1b)
			goto cancelled;

		if (key == 0x0d)
			break;
	}

	foreach (revisions, line, index)
	{
		line2JLine(line, 1, 0, 0, 1);
		trim(line, ' ');

		if (*line)
		{
			char *stamp = line;
			char *comment = "";
			char *p;

			p = strchr(stamp, '\x20');

			if (p)
			{
				*p = '\0';
				comment = p + 1;
			}
			errorCase(!lineExp("<14,09>", stamp));

			if (!*comment)
				comment = DEFAULT_COMMENT;

			cout("stamp: %s\n", stamp);
			cout("comment: %s\n", comment);

			addCwd(storeDir);
			addCwd(DIR_REVISIONS);
			addCwd(stamp);

			writeOneLine(FILE_REV_COMMENT, comment);

			unaddCwd();
			unaddCwd();
			unaddCwd();
		}
	}

cancelled:
	releaseDim(revisions, 1);
}
static void EraseDeletableRevisions(char *storeDir, int quietFlag)
{
	autoList_t *revisions;
	char *stamp;
	uint index;

	addCwd(storeDir);
	addCwd(DIR_REVISIONS);

	revisions = ls(".");
	foreach (revisions, stamp, index)
	{
		char *comment;

		addCwd(stamp);
		comment = readFirstLine(FILE_REV_COMMENT);
		unaddCwd();

#if 0 // 廃止 @ 2021.3.16
		if (!strcmp(comment, DELETABLE_COMMENT_01) || startsWith(comment, DELETABLE_COMMENT_02_START_PTN))
		{
			cout("削除予定のリビジョンです。\n");
			cout("%s\n", stamp);
			cout("%s\n", comment);
			cout("削除？\n");

			if (quietFlag || clearGetKey() != 0x1b)
			{
				cout("削除します。\n");
				semiRemovePath(stamp);
			}
		}
#endif
		memFree(comment);
	}
	releaseDim(revisions, 1);

	unaddCwd();
	unaddCwd();
}
static void EraseLostEntries(char *storeDir, int quietFlag)
{
	autoList_t *stockFiles;
	autoList_t *revisions;
	char *stamp;
	uint index;

	addCwd(storeDir);

	stockFiles = ls(DIR_FILES);
	eraseParents(stockFiles);

	addCwd(DIR_REVISIONS);

	revisions = ls(".");
	foreach (revisions, stamp, index)
	{
		autoList_t *entries;
		char *entry;
		uint entryIndex;
		int entriesModified = 0;

		addCwd(stamp);
		entries = readLines(FILE_REV_FILES);

	entriesRestart:
		foreach (entries, entry, entryIndex)
		{
			char *p = strchr(entry, '\x20');
			char *hash;

			errorCase(!p);
			*p = '\0';
			hash = strx(entry);
			*p = '\x20';

			if (findLineCase(stockFiles, hash, 1) == getCount(stockFiles)) // ? not found
			{
				cout("エントリーの実体が失われています。\n");
				cout("%s\n", stamp);
				cout("%s\n", entry);
				cout("削除？\n");

				if (quietFlag || clearGetKey() != 0x1b)
				{
					cout("削除します。\n");

					memFree((char *)desertElement(entries, entryIndex));
					entriesModified = 1;
					goto entriesRestart;
				}
			}

			if (pulseSec(2, NULL))
				cmdTitle_x(xcout("rum - E / %u(%u)_%u(%u)", entryIndex, getCount(entries), index, getCount(revisions)));
		}
		if (entriesModified)
		{
			cout("エントリー更新中...\n");
			semiRemovePath(FILE_REV_FILES);
			writeLines(FILE_REV_FILES, entries);
			cout("エントリー更新終了\n");
		}
		releaseDim(entries, 1);
		unaddCwd();
	}
	cmdTitle("rum");

	unaddCwd();
	unaddCwd();

	releaseDim(stockFiles, 1);
	releaseDim(revisions, 1);
}
static void TrimStoreDir(char *storeDir, int quietFlag)
{
	autoList_t *stockFiles;
	autoList_t *revisions;
	char *stockFile;
	char *stamp;
	uint index;

	cout("+----------------------------+\n");
	cout("| 参照されないファイルの削除 |\n");
	cout("+----------------------------+\n");
	sleep(500);

	EraseDeletableRevisions(storeDir, quietFlag);

	addCwd(storeDir);

	stockFiles = ls(DIR_FILES);
	eraseParents(stockFiles);

	revisions = ls(DIR_REVISIONS);

	foreach (revisions, stamp, index)
	{
		char *filesFile = combine(stamp, FILE_REV_FILES);
		autoList_t *stocks;
		char *line;
		uint lineIndex;

		stocks = readLines(filesFile);

		foreach (stocks, line, lineIndex)
		{
			char *p = strchr(line, '\x20');
			uint foundIndex;

			errorCase(!p);
			*p = '\0';

			errorCase(!lineExp("<32,09afAF>", line)); // 2bs

			foundIndex = findLineCase(stockFiles, line, 1);
			if (foundIndex < getCount(stockFiles))
			{
				memFree((void *)fastDesertElement(stockFiles, foundIndex));
			}

			if (pulseSec(2, NULL))
				cmdTitle_x(xcout("rum - %u / %u(%u)_%u(%u)", getCount(stockFiles), lineIndex, getCount(stocks), index, getCount(revisions)));
		}
		memFree(filesFile);
		releaseDim(stocks, 1);
	}
	cmdTitle("rum");

	addCwd(DIR_FILES);

	if (getCount(stockFiles))
	{
		foreach (stockFiles, stockFile, index)
		{
			cout("* %s\n", stockFile);
		}
		cout("削除？\n");

		if (quietFlag || clearGetKey() != 0x1b)
		{
			cout("削除します。\n");

			foreach (stockFiles, stockFile, index)
			{
				cout("! %s\n", stockFile);
				semiRemovePath(stockFile);
			}
		}
	}
	unaddCwd();
	unaddCwd();

	EraseLostEntries(storeDir, quietFlag);
}
static void FileHistory(char *storeDir, int fromLastRevisionFlag, int lastFileOnly)
{
	autoList_t *revisions;
	autoList_t *srchRevisions;
	char *stamp;
	uint index;
	autoList_t *allStock = newList();
	autoList_t *selStocks;
	char *selStock;
	uint selStockIdx;
	char *restoreRootDir;
	char *outStockTestDir = makeTempDir(NULL);

	cout("+--------------+\n");
	cout("| ファイル履歴 |\n");
	cout("+--------------+\n");
	sleep(500);

	addCwd(storeDir);
	revisions = ls(DIR_REVISIONS);
	rapidSortLines(revisions);

	if (fromLastRevisionFlag)
	{
		srchRevisions = newList();
		addElement(srchRevisions, getLastElement(revisions));
	}
	else
	{
		srchRevisions = copyAutoList(revisions);
	}

	if (lastFileOnly)
		reverseElements(revisions);

	foreach (srchRevisions, stamp, index)
	{
		char *filesFile = combine(stamp, FILE_REV_FILES);
		autoList_t *stocks;
		char *line;
		uint lineIndex;

		stocks = readLines(filesFile);

		foreach (stocks, line, lineIndex)
		{
			char *p = strchr(line, '\x20');

			errorCase(!p);
			p++;
			errorCase(!*p);

			addElement(allStock, (uint)strx(p));
		}
		memFree(filesFile);
		releaseDim(stocks, 1);
	}
	allStock = autoDistinctJLinesICase(allStock);
	selStocks = selectLines(allStock);

	if (!getCount(selStocks))
		goto noSelStocks;

	restoreRootDir = makeFreeDir();

	foreach (selStocks, selStock, selStockIdx)
	{
		autoList_t *histRevs = newList();
		autoList_t *histHashes = newList();
		char *outStock;

		cout("selStock: %s\n", selStock);

		outStock = combine(outStockTestDir, getLocal(selStock));
		outStock = toCreatablePath(outStock, selStockIdx);
		createFile(outStock);
		eraseParent(outStock);

		foreach (revisions, stamp, index)
		{
			char *filesFile = combine(stamp, FILE_REV_FILES);
			autoList_t *stocks;
			char *line;
			uint lineIndex;

			stocks = readLines(filesFile);

			foreach (stocks, line, lineIndex)
			{
				char *hash = line;
				char *stock = strchr(line, '\x20');

				errorCase(!stock);
				*stock = '\0';
				stock++;
				errorCase(!*hash);
				errorCase(!*stock);

				if (!_stricmp(stock, selStock))
				{
					int modified = !getCount(histHashes) || _stricmp(hash, getLine(histHashes, getCount(histHashes) - 1));
					char *lclStamp = getLocal(stamp);

					cout("# %s %s %s\n", lclStamp, hash, modified ? "新規または変更アリ" : "変更ナシ");

					if (modified)
					{
						addElement(histRevs, (uint)strx(lclStamp));
						addElement(histHashes, (uint)strx(hash));
					}
					break; // もう無い。
				}
			}
			memFree(filesFile);
			releaseDim(stocks, 1);

			if (lastFileOnly && getCount(histRevs))
				break;
		}
		addCwd(DIR_FILES);

		foreach (histRevs, stamp, index)
		{
			char *restoreFile;

			if (lastFileOnly)
				restoreFile = combine(restoreRootDir, outStock);
			else
				restoreFile = combine_cx(restoreRootDir, xcout("%s_%s", stamp, outStock));

			copyFile(getLine(histHashes, index), restoreFile);
			PostRestoreFile(restoreFile);
			memFree(restoreFile);
		}
		memFree(outStock);
		releaseDim(histRevs, 1);
		releaseDim(histHashes, 1);
		unaddCwd();
	}
	execute_x(xcout("START \"\" \"%s\"", restoreRootDir));
	memFree(restoreRootDir);
noSelStocks:
	releaseDim(revisions, 1);
	releaseAutoList(srchRevisions);
	releaseDim(allStock, 1);
	releaseDim(selStocks, 1);
	recurRemoveDir_x(outStockTestDir);
	unaddCwd();
}
static void RemoveLastRevIfNoMod(char *storeDir, int quietFlag, int ignoreMessage)
{
	autoList_t *revisions;
	char *stamp1;
	char *stamp2;
	char *filesFile1;
	char *filesFile2;
	char *treeFile1;
	char *treeFile2;

	addCwd(storeDir);
	addCwd(DIR_REVISIONS);
	revisions = ls(".");

	if (getCount(revisions) < 2)
	{
		cout("複数のリビジョンがありません。\n");
		goto endFunc;
	}
	eraseParents(revisions);
	rapidSortLines(revisions);

	stamp1 = getLine(revisions, getCount(revisions) - 2);
	stamp2 = getLine(revisions, getCount(revisions) - 1);

	filesFile1 = combine(stamp1, FILE_REV_FILES);
	filesFile2 = combine(stamp2, FILE_REV_FILES);
	treeFile1 = combine(stamp1, FILE_REV_TREE);
	treeFile2 = combine(stamp2, FILE_REV_TREE);

	if (
		!isSameFile(filesFile1, filesFile2) ||
		!isSameFile(treeFile1, treeFile2)
		)
	{
		cout("最後のリビジョンは更新されています。\n");
	}
	else if (!ignoreMessage && strcmp(DEFAULT_COMMENT, GetComment(storeDir, stamp2))) // ? != DEFAULT_COMMENT
	{
		cout("最後のリビジョンは更新されていませんが、コメントが記述されています。\n");
	}
	else
	{
		char *removeTargetDir = makeFullPath(stamp2);

if (ignoreMessage) cout("ignoreMessage\n"); // test
		cout("+--------------------------------------+\n");
		cout("| 最後のリビジョンは更新されていません |\n");
		cout("+--------------------------------------+\n");
		cout("> %s\n", removeTargetDir);
		cout("削除？\n");

		if (quietFlag || clearGetKey() != 0x1b)
		{
			cout("削除します。\n");
			recurRemoveDir(removeTargetDir);
			cout("削除しました。\n");
		}
		memFree(removeTargetDir);
	}
	memFree(filesFile1);
	memFree(filesFile2);
	memFree(treeFile1);
	memFree(treeFile2);
endFunc:
	releaseDim(revisions, 1);

	unaddCwd();
	unaddCwd();
}

static int RestoreListOnlyMode;

static void OneRestore(char *storeDir, char *targetStamp, char *restoreDir) // storeDir: バックアップ先, restoreDir: 未作成の出力先ディレクトリ
{
	autoList_t *dirs;
	autoList_t *stocks;
	char *line;
	uint index;

	addCwd(storeDir);
	addCwd(DIR_REVISIONS);
	addCwd(targetStamp);

	dirs = readLines(FILE_REV_TREE);
	stocks = readLines(FILE_REV_FILES);

	unaddCwd();
	unaddCwd();

	createDir(restoreDir);

	if (RestoreListOnlyMode)
	{
		autoList_t *lines = newList();

		foreach (dirs, line, index)
		{
			addElement(lines, (uint)xcout("-------------------------------- %s", line));
		}
		foreach (stocks, line, index)
		{
			addElement(lines, (uint)strx(line));
		}
		foreach (lines, line, index) // チェックのみ
		{
			errorCase(
				!lineExp("<32,--> <>", line) &&
				!lineExp("<32,09af> <>", line)
				);
		}
		rapidSort(lines, HashedPathComp);
		writeLines_xx(combine(restoreDir, "Tree.txt"), lines);
	}
	else
	{
		addCwd(restoreDir);

		foreach (dirs, line, index)
			createDir(line);

		unaddCwd();
		addCwd(DIR_FILES);

		foreach (stocks, line, index)
		{
			char *stockFile = line;
			char *file;

			file = strchr(stockFile, '\x20');
			errorCase(!file);

			file[0] = '\0';
			file++;

			errorCase(!*stockFile);
			errorCase(!*file);

			file = combine(restoreDir, file);

			cout("< %s\n", stockFile);
			cout("> %s\n", file);

			copyFile(stockFile, file);
			PostRestoreFile(file);

			memFree(file);
		}
		unaddCwd();
	}
	unaddCwd();

	releaseDim(dirs, 1);
	releaseDim(stocks, 1);
}
static void MultiRestore(char *storeDir, autoList_t *stamps)
{
	char *restoreRootDir = makeFreeDir();
	char *restoreDir;
	char *targetStamp;
	uint index;

	cout("復元先ルート: %s\n", restoreRootDir);

	foreach (stamps, targetStamp, index)
	{
		restoreDir = combine(restoreRootDir, targetStamp);
		cout("スタンプと復元先: %u(%u), %s > %s\n", index, getCount(stamps), targetStamp, restoreDir);

		OneRestore(storeDir, targetStamp, restoreDir);

		memFree(restoreDir);
	}
	execute_x(xcout("START \"\" \"%s\"", restoreRootDir));
	memFree(restoreRootDir);

	cout("複数のリビジョンを復元しました。\n");
}

static int RestoreFreeDirMode;

static void Checkout(char *dir) // dir: バックアップ先、存在するルートディレクトリではないディレクトリの絶対パス
{
	char *restoreDir;
	char *targetStamp;

	// I/O Test
	{
		errorCase(!existDir(dir));

		addCwd(dir);

		errorCase(!existDir(DIR_FILES));
		errorCase(!existDir(DIR_REVISIONS));

		unaddCwd();
	}

	restoreDir = changeExt(dir, "");
	cout("復元先: %s\n", restoreDir);

	// select stamps
	{
		autoList_t *stamps = SelectStamps(dir);

		switch (getCount(stamps))
		{
		case 0:
			releaseDim(stamps, 1);
			goto endFunc;

		case 1:
			targetStamp = getLine(stamps, 0);
			releaseAutoList(stamps);
			break;

		default:
			MultiRestore(dir, stamps);
			releaseDim(stamps, 1);
			goto endFunc;
		}
	}
	cout("復元するスタンプ: %s\n", targetStamp);

	if (existDir(restoreDir))
	{
		eraseParent(restoreDir);
		restoreDir = combine_xx(makeFreeDir(), restoreDir);
		cout("次の復元先: %s\n", restoreDir);
	}
	if (RestoreFreeDirMode)
	{
		memFree(restoreDir);
		restoreDir = makeFreeDir();
		restoreDir = combine_xc(restoreDir, "_");
		cout("更に次の復元先: %s\n", restoreDir);
	}
	OneRestore(dir, targetStamp, restoreDir);
	execute_x(xcout("START \"\" \"%s\"", c_getParent(restoreDir)));

endFunc:
	memFree(restoreDir);
}

static int WithoutExeObjMode;
static int QuietMode;
static int NoCheckCollision;
//static int NoNestingCheck; // 廃止 @ 2020.10.7

static void Commit(char *dir) // dir: バックアップ元、存在するルートディレクトリではないディレクトリの絶対パス
{
	char *storeDir;
	char *targetStamp;
	char *comment;

	storeDir = strx(dir);
	storeDir = addExt(storeDir, EXT_STOREDIR);
	cout("格納先: %s\n", storeDir);

	targetStamp = makeCompactStamp(NULL);
	cout("格納するスタンプ: %s\n", targetStamp);

	// I/O Test
	{
		if (existDir(storeDir))
		{
			addCwd(storeDir);

			errorCase(!existDir(DIR_FILES));
			errorCase(!existDir(DIR_REVISIONS));

			addCwd(DIR_REVISIONS);
			errorCase(existPath(targetStamp));
			createDir(targetStamp);
			removeDir(targetStamp);
			unaddCwd();

			unaddCwd();
		}
		else
		{
			cout("##################\n");
			cout("## 格納先未作成 ##\n");
			cout("##################\n");
			sleep(500);

			createDir(storeDir);
			removeDir(storeDir);
		}
	}

	// 廃止 @ 2020.10.7
	/*
	if (NoNestingCheck)
	{
		LOGPOS();
		goto endNestingCheck;
	}
	*/

	// 親 .rum チェック
	{
		char *parentDir = strx(dir);

		for (; ; )
		{
			parentDir = changeLocal_xc(parentDir, "");

			if (strlen(parentDir) == 2) // ? ルートに達した。
				break;

			errorCase(strlen(parentDir) <= 3); // 2bs

			parentDir = addExt(parentDir, EXT_STOREDIR);

			if (existDir(parentDir))
			{
#if 1
				error_m("親に .rum アリ");
#else // del @ 2020.10.7
				cout("####################\n");
				cout("## 親に .rum アリ ##\n");
				cout("####################\n");
				cout("%s\n", parentDir);
				sleep(500);
#endif
			}
		}
		memFree(parentDir);
	}

	// 子 .rum チェック
	{
		autoList_t *subDirs = lssDirs(dir);
		char *subDir;
		uint index;

		foreach (subDirs, subDir, index)
		{
			if (!_stricmp(EXT_STOREDIR, getExt(subDir)))
			{
#if 1
				error_m("子に .rum アリ");
#else // del @ 2020.10.7
				cout("####################\n");
				cout("## 子に .rum アリ ##\n");
				cout("####################\n");
				cout("%s\n", subDir);
				sleep(500);
#endif
			}
		}
		releaseDim(subDirs, 1);
	}
endNestingCheck:

	if (!QuietMode)
	{
		comment = inputLine();
		line2JLine(comment, 1, 0, 0, 1);
		trim(comment, ' ');

		if (!*comment)
			comment = addLine(comment, DEFAULT_COMMENT);
	}
	else
	{
		comment = GetAutoComment();

		if (!comment)
			comment = strx(DEFAULT_COMMENT);
	}
	cout("コメント: %s\n", comment);

	if (!existDir(storeDir))
	{
		cout("######################################\n");
		cout("## 続行すると格納先を新規作成します ##\n");
		cout("######################################\n");
	}
	if (!QuietMode)
	{
		cout("続行？\n");

		if (clearGetKey() == 0x1b)
		{
			ErrorLevel = 1;
			goto cancelled;
		}
		cout("続行します。\n");
	}

	if (!existDir(storeDir))
	{
		createDir(storeDir);
		execute_x(xcout("Compact.exe /C \"%s\"", storeDir)); // 圧縮

		addCwd(storeDir);
		createDir(DIR_FILES);
		createDir(DIR_REVISIONS);
		unaddCwd();
	}
	addCwd(storeDir);

	// make new revision
	{
		autoList_t *paths = lss(dir);
		autoList_t dirs;
		autoList_t files;
		autoList_t *stockFiles = newList();
		char *file;
		uint index;

		if (WithoutExeObjMode)
		{
			uint dirKilledNum = 0;

			foreach (paths, file, index)
			{
#if 0
				if (index < lastDirCount)
				{
					1; // noop
				}
				else
				{
					if (
						!_stricmp("exe", getExt(file)) ||
						!_stricmp("obj", getExt(file))
						)
					{
						file[0] = '\0';
					}
				}
#else // tmp 配下は事前にクリアするようにした。-> Counter.txt が入ってしまうので復活。
				if (index < lastDirCount)
				{
					if (startsWithICase(file, "C:\\Factory\\tmp\\")) // ? tmp 配下のディレクトリ
					{
						file[0] = '\0';
						dirKilledNum++;
					}
				}
				else
				{
					if (
						!_stricmp("exe", getExt(file)) ||
						!_stricmp("obj", getExt(file)) ||
						startsWithICase(file, "C:\\Factory\\tmp\\") // ? tmp 配下のファイル
						)
					{
						file[0] = '\0';
					}
				}
#endif
			}
			trimLines(paths);
			lastDirCount -= dirKilledNum;
		}

		dirs = gndSubElements(paths, 0, lastDirCount);
		files = gndFollowElements(paths, lastDirCount);

		sortJLinesICase(&dirs);
		sortJLinesICase(&files);

		// I/O Test
		{
			cout("読み込みテスト中\n");

			foreach (&files, file, index)
			{
				FILE *fp = fileOpen(file, "rb");

				readChar(fp);
				readChar(fp);
				readChar(fp);

				fileClose(fp);
			}
			cout("読み込みテストok!\n");
		}
		addCwd(DIR_FILES);

		foreach (&files, file, index)
		{
			char *stockFile = md5_makeHexHashFile(file);

			if (existFile(stockFile))
			{
				cout("* %s\n", file);
#if 0 // 廃止 @ 2020.10.29
				errorCase(!NoCheckCollision && !isSameFile(file, stockFile)); // ? コリジョン発生
#endif
			}
			else
			{
				/*
					stockFile を中途半端に出力してエラーとかで落ちると、ハッシュと合わないファイルができてしまいマズいので、
					確実に読み込める midFile を中継する。
				*/
				char *midFile = makeTempPath("mid");

				cout("< %s\n", file);
				cout("+ %s\n", midFile);
				cout("> %s\n", stockFile);

				copyFile(file, midFile);
				moveFile(midFile, stockFile);

				memFree(midFile);
			}
			addElement(stockFiles, (uint)stockFile);
		}
		unaddCwd();
		execute("Compact.exe /C /S:" DIR_FILES); // 再圧縮
		execute("Compact.exe /C /S:. > NUL"); // .rum 再圧縮 -- コピーするなどして圧縮が解除された場合などのため

		changeRoots(paths, dir, NULL); // リスト出力用に、相対化

		addCwd(DIR_REVISIONS);
		createDir(targetStamp);
		addCwd(targetStamp);

		if (*comment)
			writeOneLine(FILE_REV_COMMENT, comment);

		writeLines(FILE_REV_TREE, &dirs);

		// make files file
		{
			FILE *fp = fileOpen(FILE_REV_FILES, "wt");

			foreach (&files, file, index)
			{
				writeToken(fp, getLine(stockFiles, index));
				writeChar(fp, '\x20');
				writeLine(fp, file);
			}
			fileClose(fp);
		}
		releaseDim(paths, 1);
		releaseDim(stockFiles, 1);

		unaddCwd();
		unaddCwd();
	}
	unaddCwd();

cancelled:
	memFree(storeDir);
	memFree(targetStamp);
	memFree(comment);
}

static int EditCommentMode;
static int TrimStoreDirMode;
static int TrimStoreDirMode_QuietMode;
static int FileHistoryMode;
static int FileHistoryMode_FromLastRevision;
static int FileHistoryMode_LastFileOnly;
static int RemoveLastRevIfNoModMode;
static int RemoveLastRevIfNoModMode_QuietMode;
static int RemoveLastRevIfNoModMode_IgnoreMessage;
static char *InputDirExt;

static void Rum(char *dir)
{
	cmdTitle("rum");

	dir = makeFullPath(dir);
	cout("対象: %s\n", dir);

	if (InputDirExt)
	{
		dir = changeExt_xc(dir, InputDirExt);
		cout("対Ⅱ: %s\n", dir);
	}
	errorCase(isAbsRootDir(dir));
	errorCase(!existDir(dir));

	if (!_stricmp(EXT_STOREDIR, getExt(dir)))
	{
		if (EditCommentMode)
		{
			EditComment(dir);
		}
		else if (TrimStoreDirMode)
		{
			TrimStoreDir(dir, TrimStoreDirMode_QuietMode);
		}
		else if (FileHistoryMode)
		{
			FileHistory(dir, FileHistoryMode_FromLastRevision, FileHistoryMode_LastFileOnly);
		}
		else if (RemoveLastRevIfNoModMode)
		{
			RemoveLastRevIfNoMod(dir, RemoveLastRevIfNoModMode_QuietMode, RemoveLastRevIfNoModMode_IgnoreMessage);
		}
		else
		{
			Checkout(dir);
		}
	}
	else
	{
		Commit(dir);
	}
	memFree(dir);
}
int main(int argc, char **argv)
{
readArgs:
	if (argIs("/E")) // Edit comment
	{
		EditCommentMode = 1;
		goto readArgs;
	}
	if (argIs("/EE")) // erum 用
	{
		EditCommentMode = 1;
		InputDirExt = "rum";
		goto readArgs;
	}
	if (argIs("/F")) // without .exe and .obj (Factory mode)
	{
		WithoutExeObjMode = 1;
		goto readArgs;
	}
	if (argIs("/T")) // Trim store-dir
	{
		TrimStoreDirMode = 1;
		goto readArgs;
	}
	if (argIs("/TT")) // MatomeMaker 用
	{
		TrimStoreDirMode = 1;
		TrimStoreDirMode_QuietMode = 1;
		goto readArgs;
	}
	if (argIs("/HA")) // file History from All revision
	{
		FileHistoryMode = 1;
		goto readArgs;
	}
	if (argIs("/H")) // file History from last revision
	{
		FileHistoryMode = 1;
		FileHistoryMode_FromLastRevision = 1;
		goto readArgs;
	}
	if (argIs("/1A")) // last file from All revision
	{
		FileHistoryMode = 1;
		FileHistoryMode_LastFileOnly = 1;
		goto readArgs;
	}
	if (argIs("/1")) // last file from last revision
	{
		FileHistoryMode = 1;
		FileHistoryMode_FromLastRevision = 1;
		FileHistoryMode_LastFileOnly = 1;
		goto readArgs;
	}
	if (argIs("/R")) // Remove last revison if no modifications
	{
		RemoveLastRevIfNoModMode = 1;
		goto readArgs;
	}
	if (argIs("/RR")) // qrum 用
	{
		RemoveLastRevIfNoModMode = 1;
		InputDirExt = "rum";
		goto readArgs;
	}
	if (argIs("/RRR")) // qrum 用
	{
		RemoveLastRevIfNoModMode = 1;
		RemoveLastRevIfNoModMode_QuietMode = 1;
		InputDirExt = "rum";
		goto readArgs;
	}
	if (argIs("/RRRA")) // qrumall 用
	{
		RemoveLastRevIfNoModMode = 1;
		RemoveLastRevIfNoModMode_QuietMode = 1;
		RemoveLastRevIfNoModMode_IgnoreMessage = 1;
		InputDirExt = "rum";
		goto readArgs;
	}
	if (argIs("/Q")) // Quiet mode
	{
		QuietMode = 1;
		goto readArgs;
	}
	if (argIs("/D")) // restore free Dir mode
	{
		RestoreFreeDirMode = 1;
		goto readArgs;
	}
	if (argIs("/L")) // restore List only mode
	{
		RestoreListOnlyMode = 1;
		goto readArgs;
	}
	if (argIs("/-COLL"))
	{
#if 1
		error_m("/-COLL オプションは廃止されました。");
#else // 廃止 @ 2020.10.29
		NoCheckCollision = 1;
		goto readArgs;
#endif
	}
	if (argIs("/-NCHK"))
	{
#if 1
		error_m("/-NCHK オプションは廃止されました。");
#else // 廃止 @ 2020.10.7
		NoNestingCheck = 1;
		goto readArgs;
#endif
	}
	if (argIs("/C"))
	{
		char *comment;

		if (hasArgs(1))
			writeOneLineNoRet_b(AUTO_COMMENT_FILE, nextArg());
		else
			editTextFile(AUTO_COMMENT_FILE);

		comment = GetAutoComment();
		cout("自動コメント: %s\n", comment ? comment : "<NULL>");

		goto endProc;
	}
	if (argIs("/C-"))
	{
		removeFileIfExist(AUTO_COMMENT_FILE);
		cout("自動コメントを削除しました。\n");
		goto endProc;
	}
	if (argIs("/XR")) // rrum 用
	{
		InputDirExt = "rum";
		goto readArgs;
	}
	Rum(hasArgs(1) ? nextArg() : c_dropDir());

endProc:
	termination(ErrorLevel);
}
