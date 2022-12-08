/*
	fileStore.exe [/T TITLE]? [/FC FORE-COLOR]? [/F FORE-COLOR-FILE]? [/B BACK-COLOR]? [/S SUBMIT-TEXT]?
	              [/UX UPLOAD-DIR-SIZE-MAX]? [/UFX UPLOAD-FILE-SIZE-MAX]? [/UD UPLOAD-DIR]? [/P PORT]? [/C CONNECT-MAX]? [/TO TIMEOUT]? [/L]? [/H HOST-NAME]?

		TITLE            == ページのタイトル
		FORE-COLOR       == ページの文字色          例: White, Blue, #ffffff, #0000ff
		FORE-COLOR-FILE  == ページのリンクの文字色  例: White, Blue, #ffffff, #0000ff
		BACK-COLOR       == ページの背景色          例: White, Blue, #ffffff, #0000ff
		SUBMIT-TEXT      == 送信ボタンのテキスト
		UPLOAD-DIR-SIZE-MAX  == サーバーの容量 (バイト)              デフォ: 10GB
		UPLOAD-FILE-SIZE-MAX == 一つのファイルの最大サイズ (バイト)  デフォ: 701MB (Content-Lengthであることに注意。大きくても 2GB 弱が無難)
		UPLOAD-DIR       == ファイル格納先       デフォ: 適当な場所
		PORT             == 待ち受けポート番号   デフォ: 80
		CONNECT-MAX      == 最大接続数           デフォ: 10
		TIMEOUT          == 通信タイムアウト(秒) デフォ: 86400 (24h)
		/L               == ファイルロック
		HOST-NAME        == リクエストを許可するホスト名  デフォ: 無指定(全てのホスト名を許可)

	最悪のメモリ使用量
		大きなメモリブロックは確保しないので、メモリの空きを憂慮しなくても良い。

	最悪のディスク使用量
		アップロード許可 (/L 無し)
			UPLOAD-DIR-SIZE-MAX + UPLOAD-FILE-SIZE-MAX * (CONNECT-MAX + 1) [byte]
			これは以下の場合
				サーバー満杯
				接続数が最大値
				一つの接続が以下の状態
					最大ファイルサイズのアップロード完了直後(パート分解中)
				他の接続が以下のいずれか
					最大ファイルサイズのアップロード完了直前
					最大ファイルサイズのダウンロード中
			但し、最大ファイルサイズより大きなファイルをファイル格納先に手動で置くなどした場合、これをダウンロードされるとこの限りではない。
		アップロード不許可 (/L 有り)
			大きな一時ファイルは作成しないので、ディスクの空きを憂慮しなくても良い。
			UPLOAD-DIR-SIZE-MAX を超えてアップロード先にファイルを置ける。(チェックされないため)

	ファイル格納先の要件 (ファイル格納先を指定した場合)
		終了時に削除しない。
		最初から存在するファイルはアップロードによって削除する場合がある。
		サブディレクトリがあってはならない。

	http://localhost/                           ファイルリスト取得
	http://localhost/FILE                       ダウンロード
	http://localhost/*delete:FILE               削除
	http://localhost/*rename:OLD-FILE:NEW-FILE  リネーム

		ダウンロード・削除・リネームのときファイル名は URL-encode してね。
*/

#include "C:\Factory\Common\Options\SockServerTh.h"
#include "libs\http\MultiPart.h"

#define FAVICON_FILE "favicon.ico"
#define HTTP_NEWLINE "\r\n"
#define UPLOADFILEMAX 5000

static char *Title = "fileStore";
static char *ForeColor = "#556677";
static char *ForeColorFile = "#002255";
static char *BackColor = "#ddeeff";
static char *SubmitText = "アップロード";

#define DEF_CONNECTMAX 10
#define DEF_UPLOADFILESIZEMAX (1024 * 1024 * 701)
#define DEF_UPLOADDIRSIZEMAX ((uint64)1024 * 1024 * 1024 * 10)
#define DEF_TIMEOUT 86400

static uint64 UploadDirSizeMax = DEF_UPLOADDIRSIZEMAX;
static char *UploadDir; // フルパス
static uint Timeout = DEF_TIMEOUT;
static uint FileLock;

static void L2AsciiFile(char *file)
{
#if 0
	line2fsym(file);
#else
	if (!lineExp("<1,,-.__09AZaz>", file) ||
		lineExp(".<>", file) ||
		lineExp("<>.", file) ||
		lineExp("<>..<>", file))
	{
		char *xp = getExtWithDot(file);

		line2csym(file);

		if (file < xp && xp[1])
		{
			*xp = '.';
		}
	}
#endif
}
static uint64 GetTotalFileSize(autoList_t *files)
{
	char *file;
	uint index;
	uint64 totalSize = 0;

	foreach (files, file, index)
	{
		totalSize += getFileSize(file);
	}
	return totalSize;
}
static autoList_t *GetUploadFiles(void)
{
	autoList_t *files = ls(UploadDir);
	char *file;
	uint index;
	autoList_t *lines = newList();

	foreach (files, file, index)
	{
		updateFindData(file);
		addElement(lines, (uint)xcout("%20I64u%s", lastFindData.time_write, file));
	}
	releaseDim(files, 1);

	rapidSortLines(lines);
	reverseElements(lines); // 新しい順

	foreach (lines, file, index)
		copyLine(file, file + 20);

	files = lines;
	return files;
}
static void CheckUploadDirOverflow(void)
{
	autoList_t *files = GetUploadFiles();

	while (UPLOADFILEMAX < getCount(files) ||
		UploadDirSizeMax < GetTotalFileSize(files))
		removeFile_x((char *)unaddElement(files));

	releaseDim(files, 1);
}

static uint UDS_FileCount;
static uint64 UDS_DirSize;

static void UpdateUploadDirStatus(void)
{
	autoList_t *files = ls(UploadDir);
	char *file;
	uint index;

	UDS_FileCount = getCount(files);
	UDS_DirSize = 0;

	foreach (files, file, index)
		UDS_DirSize += getFileSize(file);

	releaseDim(files, 1);
}
static char *GetLineDivList(void)
{
	autoList_t *files = ls(UploadDir);
	char *file;
	uint index;
	autoList_t *divList = newList();
	char *sDivList;

	foreach (files, file, index)
	{
		char *strStamp;
		char *strSize;
		char *href;
		char *line;

		updateFindData(file);

		strStamp = makeJStamp(getStampDataTime(lastFindData.time_write), 0);
		strSize = xcout("%I64u", getFileSize(file));
		strSize = thousandComma(strSize);
		href = getLocal(file);
		href = httpUrlEncoder(href);

		line = xcout("<div class=\"upfile-row\">%s　<a class=\"upfile-link\" href=\"%s\">%s</a>　%s bytes</div>", strStamp, href, getLocal(file), strSize);

		memFree(strStamp);
		memFree(strSize);
		memFree(href);

		addElement(divList, (uint)line);
	}
	releaseDim(files, 1);

	rapidSortLines(divList);
	reverseElements(divList); // 新しい順

	sDivList = untokenize(divList, "\n");
	releaseDim(divList, 1);
	return sDivList;
}
static char *MakeFileListHtmlFile(void)
{
	char *file = makeTempPath("html");
	autoList_t *lines;
	char *line;

	// Load .html_
	{
		char *file = getSelfFile();

		file = changeExt(file, "html_");
		lines = readLines(file);
		memFree(file);
	}
	line = untokenize(lines, "\n");

	line = replaceLine(line, "*title*", Title, 1);
	line = replaceLine(line, "*fore-color*", ForeColor, 1);
	line = replaceLine(line, "*fore-color-file*", ForeColorFile, 1);
	line = replaceLine(line, "*back-color*", BackColor, 1);
	line = replaceLine(line, "*submit*", SubmitText, 1);

	// optional
	UpdateUploadDirStatus();
	line = replaceLine(line, "*time-stamp*", c_makeJStamp(NULL, 0), 1);
	line = replaceLine_cx(line, "*file-num*", xcout("%u", UDS_FileCount), 1);
	line = replaceLine_cx(line, "*file-let*", xcout("%u", UPLOADFILEMAX - UDS_FileCount), 1); // overflow-able
	line = replaceLine_cx(line, "*file-max*", xcout("%u", httpMultiPartContentLenMax), 1);
	line = replaceLine_cx(line, "*dir-size*", xcout("%I64u", UDS_DirSize), 1);
	line = replaceLine_cx(line, "*dir-free*", xcout("%I64u", FileLock ? 0ui64 : UploadDirSizeMax - UDS_DirSize), 1); // overflow-able

	// Replace div-list
	{
		char *sDivList = GetLineDivList();

		line = replaceLine(line, "*div-list*", sDivList, 1);
		memFree(sDivList);
	}
	writeOneLine(file, line);
	memFree(line);
	return file;
}
static char *MakeRefreshHtmlFile(char *message)
{
	char *file = makeTempPath("html");

	writeOneLine_cx(file, xcout(
		"<html>\n"
		"<head>\n"
		"<meta http-equiv=\"refresh\" content=\"5; url=/\"/>\n"
		"</head>\n"
		"<body>\n"
		"<a href=\"/\">%s</a>\n"
		"</body>\n"
		"</html>"
		,message
		));

	return file;
}
static char *GetBeer(void)
{
	static char *beers[] =
	{
		"Guinness",
		"HUB-ALE",
		"SINGHA",
		"Chimay",
		"Duvel",
		"Hoegaarden",
		"Heineken",
		"HEARTLAND",
	};
	static uint index;
	char *beer;

	beer = beers[index];
	index = (index + 1) % lengthof(beers);
	return beer;
}
static int IsAbnormalExt(char *ext)
{
	return
		!_stricmp(ext, "BAT") ||
		!_stricmp(ext, "VBS") ||
		!_stricmp(ext, "CMD") ||
		!_stricmp(ext, "COM") ||
		!_stricmp(ext, "EXE") ||
		!_stricmp(ext, "INI");
}
static void PerformTh(int sock, char *strip)
{
	SockStream_t *i = CreateSockStream(sock, Timeout);
	char *header;
	autoList_t *parts;
	char *dlfile;
	int dlfilekeep = 0;

	cout("%s Co %d %s ", c_makeCompactStamp(NULL), sock, strip);

	parts = httpRecvRequestMultiPart(i, &header);

	{
		char *jh = strx(header);

		line2JLine(jh, 1, 0, 0, 1);
		cout("%s\n", jh);
		memFree(jh);
	}

	if (!_stricmp(c_httpGetPartLine(parts, "upload"), "upload"))
	{
		httpPart_t *part = httpGetPart(parts, "file");
		char *upfile;
		int errored = 0;
		char *strw;

		upfile = part->LocalFile;
		upfile = getLocal(upfile);
		upfile = lineToFairLocalPath(upfile, strlen(UploadDir));

		cout("[%d] upfile: %s\n", sock, upfile);

		if (!strcmp(upfile, "_") || !_stricmp(upfile, FAVICON_FILE)) // ? ファイルリスト取得時と一緒になってしまうので "_" は回避 + favicon アップ禁止
		{
			upfile[0] = '$';
		}
		if (IsAbnormalExt(getExt(upfile)))
		{
			upfile = addChar(upfile, '_');
		}
		upfile = combine_cx(UploadDir, upfile);

		if (getFileSize(part->BodyFile) != 0) // 0 バイトは除外 <- (アップロード連打, Overflow) 対策を兼ねる。
		{
			errorCase(FileLock); // ここまで到達しない。

			if (existPath(upfile)) // ファイル名が被ったら、既存のファイルを退避する。
			{
				char *escfile = strx(upfile);

				escfile = toCreatablePath(escfile, UPLOADFILEMAX + 10);
				moveFile(upfile, escfile);
				memFree(escfile);
			}
			moveFile(part->BodyFile, upfile);
			createFile(part->BodyFile);

			CheckUploadDirOverflow();
		}
		else
		{
			errored = 1;
		}

		/*
			共有フォルダからファイルに触れるようにする。
		*/
		addCwd(UploadDir);
		coExecute_x(xcout("ECHO Y|CACLS \"%s\" /P Users:F Guest:F", getLocal(upfile)));
		unaddCwd();

		memFree(upfile);

		dlfile = MakeRefreshHtmlFile(strw = xcout(
			"アップロードは、たぶん「%s」しました。ジャンプしない場合はここをクリックして下さい。%s"
			,errored ? "失敗" : "成功"
			,FileLock ? "(FileLock)" : ""
			));
		memFree(strw);
		SetSockStreamTimeout(i, 30);
	}
	else
	{
		httpDecode_t dec;
		char *lastLPath;

		httpDecodeHeader(header, &dec);

		lastLPath = getCount(dec.DirList) ? (char *)getLastElement(dec.DirList) : "_";
		lastLPath = strx(lastLPath);

		httpDecodeHeaderFree(&dec);

		if (lastLPath[0] == '*') // ? 特殊コマンド
		{
			autoList_t *specOpts = tokenize(lastLPath + 1, ':');

			dlfile = NULL;

			if (getCount(specOpts))
			{
				char *specCmd = (char *)desertElement(specOpts, 0);

				if (FileLock)
				{
					dlfile = MakeRefreshHtmlFile("全てのファイルはロックされています。");
				}
				else if (!_stricmp(specCmd, "rename") && getCount(specOpts) == 2) // 名前の変更
				{
					char *file1 = refLine(specOpts, 0);
					char *file2 = refLine(specOpts, 1);

					file1 = lineToFairLocalPath(file1, strlen(UploadDir));
					file2 = lineToFairLocalPath(file2, strlen(UploadDir));

					if (!strcmp(file2, "_") || !_stricmp(file2, FAVICON_FILE)) // 回避
					{
						file2[0] = '$';
					}
					file1 = combine_cx(UploadDir, file1);
					file2 = combine_cx(UploadDir, file2);

					if (existFile(file1) && !existPath(file2))
					{
						moveFile(file1, file2);
						dlfile = MakeRefreshHtmlFile("名前を変更しました。");
					}
					memFree(file1);
					memFree(file2);
				}
				else if (!_stricmp(specCmd, "remove") && getCount(specOpts) == 1) // rename に似てるから駄目
				{
					dlfile = MakeRefreshHtmlFile("remove -> delete");
				}
				else if (!_stricmp(specCmd, "delete") && getCount(specOpts) == 1) // ファイル削除
				{
					char *killfile = refLine(specOpts, 0);

					killfile = combine_cx(UploadDir, lineToFairLocalPath(killfile, strlen(UploadDir)));

					if (existFile(killfile))
					{
						removeFile(killfile);
						dlfile = MakeRefreshHtmlFile("ファイルを削除しました。");
					}
					memFree(killfile);
				}
				memFree(specCmd);
			}
			releaseDim(specOpts, 1);

			if (!dlfile)
				dlfile = MakeRefreshHtmlFile("コマンドエラー、何も実行しませんでした。");

			SetSockStreamTimeout(i, 30);
		}
		else
		{
			char *upfile = lineToFairLocalPath(lastLPath, strlen(UploadDir));

			upfile = combine_cx(UploadDir, upfile);

			if (existFile(upfile))
			{
				char *savefile = getLocal(upfile);

				savefile = strx(savefile);
				L2AsciiFile(savefile);
				i->Extra.SaveFile = savefile;

				if (!FileLock)
				{
					dlfile = makeTempPath(getExt(upfile));
					copyFile(upfile, dlfile);
				}
				else
				{
					dlfile = strx(upfile);
					dlfilekeep = 1;
				}
				cout("dlfile: %s\n", dlfile);
			}
			else
			{
				dlfile = MakeFileListHtmlFile();
				SetSockStreamTimeout(i, 180);
			}
			memFree(upfile);
		}
		memFree(lastLPath);
	}
	i->Extra.ServerName = xcout("fileStore %s-b %s", FileLock ? "-L " : "", GetBeer());
	httpSendResponseFile(i, dlfile);
	memFree(i->Extra.ServerName);
	i->Extra.ServerName = NULL;

	if (!dlfilekeep)
		removeFile(dlfile);
	else
		cout("keep: %s\n", dlfile);

	memFree(dlfile);
	memFree(header);
	httpReleaseParts(parts);

	if (i->Extra.SaveFile)
	{
		memFree(i->Extra.SaveFile);
		i->Extra.SaveFile = NULL;
	}
	ReleaseSockStream(i);

	cout("%s dC %d\n", c_makeCompactStamp(NULL), sock);
}
static int IdleTh(void)
{
	while (hasKey())
	{
		if (getKey() == 0x1b)
		{
			return 0;
		}
		cout("ESC to end.\n");
	}
	return 1;
}
int main(int argc, char **argv)
{
	uint portno = 80;
	uint connectmax = DEF_CONNECTMAX;

	httpMultiPartContentLenMax = DEF_UPLOADFILESIZEMAX;

readArgs:
	if (argIs("/T")) // Title
	{
		Title = nextArg();
		goto readArgs;
	}
	if (argIs("/FC")) // Fore Color
	{
		ForeColor = nextArg();
		goto readArgs;
	}
	if (argIs("/F")) // fore color File
	{
		ForeColorFile = nextArg();
		goto readArgs;
	}
	if (argIs("/B")) // Back color
	{
		BackColor = nextArg();
		goto readArgs;
	}
	if (argIs("/S")) // Submit text
	{
		SubmitText = nextArg();
		goto readArgs;
	}

	if (argIs("/UX")) // Upload dir size maX
	{
		UploadDirSizeMax = toValue64(nextArg());
		goto readArgs;
	}
	if (argIs("/UFX")) // Upload File size maX
	{
		httpMultiPartContentLenMax = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/UD")) // Upload Dir
	{
		UploadDir = nextArg();
		goto readArgs;
	}

	if (argIs("/P")) // Port
	{
		portno = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/C")) // Connect max
	{
		connectmax = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/TO")) // Time-Out
	{
		Timeout = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/L")) // File lock
	{
		FileLock = 1;
		goto readArgs;
	}
	if (argIs("/H")) // File lock
	{
		httpRecvRequestHostValue = nextArg();
		goto readArgs;
	}

	cmdTitle_x(xcout("fileStore %u (%s)", portno, httpRecvRequestHostValue ? httpRecvRequestHostValue : "*"));

	if (FileLock)
		httpMultiPartContentLenMax = 0;

	if (UploadDir)
	{
		UploadDir = makeFullPath(UploadDir);

		errorCase(!existDir(UploadDir));

		{
			autoList_t *dirs = lsDirs(UploadDir);

			errorCase(getCount(dirs)); // ? サブディレクトリがある。
			releaseAutoList(dirs);
		}

		sockServerTh(PerformTh, portno, connectmax, IdleTh);
	}
	else
	{
		UploadDir = makeTempDir("fileStore");

		sockServerTh(PerformTh, portno, connectmax, IdleTh);

		execute_x(xcout("RD /S /Q \"%s\"", UploadDir));
		memFree(UploadDir);
	}
}
