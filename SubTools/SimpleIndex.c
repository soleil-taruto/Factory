/*
	SimpleIndex.exe [/T タイトル] [/L リンク色] [/X テキスト色] [/B 背景色] [/P ルートの親リンク] [/-I] [/-M] [/-S] [/RN] ルートDIR

		/RN ... robots=noindex にする。$(ROBOTS) の記述は必要
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\URL.h"
#include "libs\md5Cache.h"

#define DEF_INDEXTEMPLATE "C:\\Factory\\Resource\\index.html.txt"
//#define INDEXTEMPLATE "_index.html_" // 廃止 @ 2022.8.13
#define HEADERFILE "_header.html_"
#define FOOTERFILE "_footer.html_"
#define INDEXFILE "index.html"

static char *Title = "ファイル一覧";
static char *LinkColor = "#ffffff";
static char *TextColor = "#b5cb8f";
static char *BackColor = "#556b2f";
static char *RootParentHRef;
static int ImageTagDisabled;
static int MD5Disabled;

#define HIDDEN_FILE_TRAILER " (this file will be deleted soon)"
//#define HIDDEN_FILE_TRAILER " (削除予定)"
//#define HIDDEN_FILE_TRAILER " (HIDDEN)"

static int MakeIndexMaxDepth = IMAX;

static void MakeIndex(char *, uint, int);

static int IsSimpleName(char *localPath)
{
	return lineExp("<1,,-.__09AZaz>", localPath) &&
		!lineExp("<>..<>", localPath) &&
		!lineExp(".<>", localPath) &&
		!lineExp("<>.", localPath);

	// "<>." というローカル名は作成できないぽい。。。
}
static char *MkDivLine(char *href, char *lref, char *trailer)
{
	if (href)
		return xcout("<div><a href=\"%s\">%s</a>%s</div>", c_urlEncoder(href), lref, trailer);
	else
		return xcout("<div>%s%s</div>", lref, trailer);
}
static char *S_PC_MaskPath(char *path) // ret: strx()
{
	path = strx(path);

	replaceChar(path, '_', '\x01');

	return path;
}
static sint S_PathComp(char *path1, char *path2)
{
	int ret;

	path1 = S_PC_MaskPath(path1);
	path2 = S_PC_MaskPath(path2);

	ret = mbs_stricmp(path1, path2);

	memFree(path1);
	memFree(path2);

	return ret;
}
static sint S_DirFileComp(char *path1, char *path2)
{
	int d1 = existDir(path1);
	int d2 = existDir(path2);

	if (d1 && !d2) return -1;
	if (d2 && !d1) return 1;

	return S_PathComp(path1, path2);
}
static char *MakeDivList(uint depth, int noIndex)
{
	static uint funcEnteredCount;
	autoList_t *paths = ls(".");
	char *path;
	uint index;
	uint dircnt;
	autoList_t *divs = newList();
	char *divlist;

	funcEnteredCount++;

	dircnt = lastDirCount;

	if (depth)
//		addElement(divs, (uint)MkDivLine("../" INDEXFILE, "&lt;parent directory&gt;", "")); // del @ 2023.1.28
		addElement(divs, (uint)MkDivLine("../",           "&lt;parent directory&gt;", ""));
	else if (RootParentHRef)
		addElement(divs, (uint)MkDivLine(RootParentHRef, "&lt;return&gt;", ""));

	rapidSort(paths, (sint (*)(uint, uint))S_DirFileComp);

	foreach (paths, path, index)
	{
		char *href;
		char *lref;
		char *trailer;
		uint64 size;

		cmdTitle_x(xcout("SimpleIndex - %u (%u) %u PCT = %u / %u", funcEnteredCount, depth, (uint)(index * 100.0 / getCount(paths)), index, getCount(paths)));

		if (!IsSimpleName(getLocal(path)))
			cout("URLに適さない名前: %s\n", path);

		path = getLocal(path);

		if (index < dircnt) // ? dir
		{
			int hiddenItem = path[0] == '_'; // "_" で始まるローカルディレクトリは隠す。(項目を表示しない) かつ robots=noindex にする。

			MakeIndex(path, depth + 1, hiddenItem || noIndex);

			if (hiddenItem)
				goto nextPath;

//			href = xcout("%s/" INDEXFILE, path); // del @ 2023.1.28
			href = xcout("%s/", path);
			lref = xcout("%s", path);
			trailer = strx("");
		}
		else if (
//			!_stricmp(INDEXTEMPLATE, path) || // 廃止 @ 2022.8.13
			!_stricmp(HEADERFILE, path) ||
			!_stricmp(FOOTERFILE, path) ||
			!_stricmp(INDEXFILE, path)
			)
		{
			goto nextPath;
		}
		else // ? file
		{
			char *lsize;
			char *hash;
			autoBlock_t *pab;
			char *stamp;

			size = getFileSize(path);
			lsize = xcout("%I64u", size);
			lsize = thousandComma(lsize);

			if (!MD5Disabled)
			{
				pab = md5Cache_makeHashFile(path);
				hash = makeHexLine(pab);
				releaseAutoBlock(pab);
			}
			else
			{
				hash = strx("unknown (not calculated)");
			}

			updateFindData(path);
			stamp = makeStamp(lastFindData.time_write);

			href = strx(path);
			lref = xcout("%s", path);
			trailer = xcout(" %s bytes %s md5: %s", lsize, stamp, hash);

			memFree(lsize);
			memFree(hash);
			memFree(stamp);
		}

		{
			char *prm_href = href;

			if (size == 0)
				prm_href = NULL;

			addElement(divs, (uint)MkDivLine(prm_href, lref, trailer));
		}

		if (!ImageTagDisabled)
		{
			char *ext = getExt(href);

			if (
				!_stricmp("BMP", ext) ||
				!_stricmp("GIF", ext) ||
				!_stricmp("JPG", ext) ||
				!_stricmp("JPEG", ext) ||
				!_stricmp("PNG", ext)
				)
			{
				char *div = xcout("<div><a href=\"%s\"><img src=\"%s\"/></a></div>", c_urlEncoder(href), href);

				addElement(divs, (uint)div);
			}
			else if (
				!_stricmp("avi", ext) ||
				!_stricmp("mp4", ext) ||
				!_stricmp("mpeg", ext) ||
				!_stricmp("mpg", ext) ||
				!_stricmp("webm", ext)
				)
			{
				char *div = xcout("<div><video src=\"%s\" controls style=\"max-height: 75vh;\"></video></div>", href);

				addElement(divs, (uint)div);
			}
			else if (
				!_stricmp("mid", ext) ||
				!_stricmp("midi", ext) ||
				!_stricmp("mp3", ext) ||
				!_stricmp("wav", ext) ||
				!_stricmp("wma", ext)
				)
			{
				char *div = xcout("<div><audio src=\"%s\" controls></audio></div>", href);

				addElement(divs, (uint)div);
			}
		}

		memFree(href);
		memFree(lref);
		memFree(trailer);

	nextPath:
		;
	}
	releaseDim(paths, 1);

	divlist = untokenize(divs, "\n");
	releaseDim(divs, 1);
	return divlist;
}
static char *GetTemplate(char *file) // ret: bind
{
	static char *text;

	memFree(text);

	if (existFile(file))
		text = readText(file);
	else
		text = strx("");

	return text;
}
static void MakeIndex(char *dir, uint depth, int noIndex)
{
	autoList_t *lines;
	char *lhtml;
	char *divlist;

	if (MakeIndexMaxDepth < depth)
		return;

	addCwd(dir);

	// 追加 @ 2022.8.13
	// indexファイル(index.html)が既に存在する場合、
	// テンプレートの最初の行が一致しなければ
	// ユーザーが作成したファイルと見なして更新しない。
	//
	if (existFile(INDEXFILE))
	{
		autoList_t *lines1 = readLines(INDEXFILE);
		autoList_t *lines2 = readLines(DEF_INDEXTEMPLATE);
		char *line1;
		char *line2;

		line1 = getLine(lines1, 0);
		line2 = getLine(lines2, 0);

		if (strcmp(line1, line2))
		{
			cout("インデックスファイルの最初の行が一致しないためユーザー作成と見なし更新しない。\n");
			cout("dir: %s\n", dir);

			releaseDim(lines1, 1);
			releaseDim(lines2, 1);

			goto endFunc;
		}
		releaseDim(lines1, 1);
		releaseDim(lines2, 1);
	}

//	lines = readLines(existFile(INDEXTEMPLATE) ? INDEXTEMPLATE : DEF_INDEXTEMPLATE); // 廃止 @ 2022.8.13
	lines = readLines(DEF_INDEXTEMPLATE);

	lhtml = untokenize(lines, "\n");
	lhtml = addChar(lhtml, '\n');

	lhtml = replaceLine(lhtml, "$(ROBOTS)", noIndex ? "noindex" : "all", 1);
	lhtml = replaceLine(lhtml, "$(TITLE)", Title, 1);
	lhtml = replaceLine(lhtml, "$(LINK-COLOR)", LinkColor, 1);
	lhtml = replaceLine(lhtml, "$(TEXT-COLOR)", TextColor, 1);
	lhtml = replaceLine(lhtml, "$(BACK-COLOR)", BackColor, 1);
	lhtml = replaceLine(lhtml, "$(HEADER)", GetTemplate(HEADERFILE), 1);
	lhtml = replaceLine(lhtml, "$(FOOTER)", GetTemplate(FOOTERFILE), 1);

	divlist = MakeDivList(depth, noIndex);

	lhtml = replaceLine(lhtml, "$(DIV-LIST)", divlist, 1);

	{
		char *lhtmlOld = existFile(INDEXFILE) ? readText(INDEXFILE) : NULL;

		if (!lhtmlOld || strcmp(lhtmlOld, lhtml)) // ? indexファイル新規作成 || indexファイルの内容が更新された。
		{
#if 0
			// test test test test test -- lhtml, lhtmlOld 比較用
			{
				static uint out_no;

				out_no++;

				writeOneLineNoRet_b_xc(getOutFile_x(xcout("%04u_dir.txt", out_no)), dir);
				writeOneLineNoRet_b_xc(getOutFile_x(xcout("%04u_new.txt", out_no)), lhtml);
				writeOneLineNoRet_b_xc(getOutFile_x(xcout("%04u_old.txt", out_no)), lhtmlOld ? lhtmlOld : "<NULL>");
			}
#endif

			writeOneLineNoRet(INDEXFILE, lhtml);
		}
		memFree(lhtmlOld);
	}

	releaseDim(lines, 1);
	memFree(lhtml);
	memFree(divlist);

endFunc:
	unaddCwd();
}

int main(int argc, char **argv)
{
	int noIndex = 0;
	char *dir;

readArgs:
	if (argIs("/T"))
	{
		Title = nextArg();
		goto readArgs;
	}
	if (argIs("/L"))
	{
		LinkColor = nextArg();
		goto readArgs;
	}
	if (argIs("/X"))
	{
		TextColor = nextArg();
		goto readArgs;
	}
	if (argIs("/B"))
	{
		BackColor = nextArg();
		goto readArgs;
	}
	if (argIs("/P"))
	{
		RootParentHRef = nextArg();
		goto readArgs;
	}
	if (argIs("/-I"))
	{
		ImageTagDisabled = 1;
		goto readArgs;
	}
	if (argIs("/-M"))
	{
		MD5Disabled = 1;
		goto readArgs;
	}
	if (argIs("/-S"))
	{
		MakeIndexMaxDepth = 0;
		goto readArgs;
	}
	if (argIs("/RN"))
	{
		noIndex = 1;
		goto readArgs;
	}

//	dir = hasArgs(1) ? nextArg() : c_dropDir();
	dir = nextArg();
	errorCase(!existDir(dir));

	errorCase_m(hasArgs(1), "不明なコマンド引数");

	MakeIndex(dir, 0, noIndex);

	cmdTitle("SimpleIndex");
}
