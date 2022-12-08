/*
	主なline2*系                                                               長さの変化  空文字列回避  用途？
		line2csym()            Cシンボルのような文字列                         なし        しない        安易なパス化
		line2fsym()            Cシンボルのようなファイル名                     なし        しない        安易なパス化
		lineToPrintLine()      画面に表示可能な文字列                          なし (*1)   しない        画面にログとして出す前のフィルタ (*2)
		line2JLine()           画面に表示可能な日本語文字列                    あり        しない        自由文書のフィルタ
		lineToJDoc()           画面に表示可能なトークナイズされた日本語文字列  あり        する          自由文書をプログラム的に処理する前のフィルタ
		lineToFairLocalPath()  ローカルファイル名として使用可能な文字列        あり        する          パス化

		識別子の... 2 -> 確保(再配置)しない, To -> 確保する

		(*1) insRet != 0 のときは延長あり。
		(*2) バイナリが混じりうるテキスト、信用していないネットワークからのHTTPデータなどを想定。
*/

#include "all.h"

#define DEFCHR '?'
#define COLDIG_DLMTR '~'

char *incrementColumnDigits(char *column, char *digits) // ret: strr(column)
{
	char *p = strchr(column, '\0');
	char *enddp;

	errorCase(!digits[0]);
	enddp = strchr(digits, '\0') - 1;

	while (column < p)
	{
		int d = *--p;
		char *dp;

		dp = strchr(digits, d);

		if (!dp)
		{
			if (!p[1])
				column = addChar(column, COLDIG_DLMTR); // column の最後に digits が無い -> column と digits のデリミタを付加

			break;
		}
		if (dp < enddp)
		{
			*p = dp[1];
			goto endfunc;
		}
		*p = digits[0];
	}
	column = addChar(column, digits[0]);

	for (p = strchr(column, '\0') - 1; column < p; p--)
		if (p[-1] != digits[0])
			break;

	*p = digits[1];

endfunc:
	return column;
}
char *incrementPathDigits(char *path, char *digits) // ret: strr(path)
{
	char *ext = strx(getExt(path));
	char *path2 = path;

	path = changeExt(path, "");
	path = incrementColumnDigits(path, digits);
	path = addExt(path, ext);

	memFree(ext);
	memFree(path2);
	return path;
}
char *incrementColumn(char *column)
{
	return incrementColumnDigits(column, decimal);
}
char *incrementPath(char *path)
{
	return incrementPathDigits(path, decimal);
}

/*
	toCreatablePath(strx("abc123000.txt"), 0);    -> 想定される戻り値 : "abc123000.txt"
	toCreatablePath(strx("abc123000.txt"), 1);    -> 想定される戻り値 : "abc123000.txt" ～ "abc123001.txt"
	toCreatablePath(strx("abc123000.txt"), 999);  -> 想定される戻り値 : "abc123000.txt" ～ "abc123999.txt"
	toCreatablePath(strx("abc123000.txt"), 1000); -> 想定される戻り値 : "abc123000.txt" ～ "abc124000.txt"
*/
char *toCreatablePath(char *path, uint faultCountMax) // ret: strr(path)
{
	uint faultCount = 0;

	while (existPath(path))
	{
		errorCase(faultCountMax <= faultCount);
		faultCount++;

		path = incrementPath(path);
	}
	return path;
}
/*
	toCreatableTildaPath(strx("abc123000.txt"), 0);    -> 想定される戻り値 : "abc123000.txt", "abc123000~0.txt"
	toCreatableTildaPath(strx("abc123000.txt"), 1);    -> 想定される戻り値 : "abc123000.txt", "abc123000~0.txt" ～ "abc123000~1.txt"
	toCreatableTildaPath(strx("abc123000.txt"), 999);  -> 想定される戻り値 : "abc123000.txt", "abc123000~0.txt" ～ "abc123000~999.txt"
	toCreatableTildaPath(strx("abc123000.txt"), 1000); -> 想定される戻り値 : "abc123000.txt", "abc123000~0.txt" ～ "abc123000~1000.txt"
*/
char *toCreatableTildaPath(char *path, uint faultCountMax) // ret: strr(path)
{
	if (existPath(path))
	{
		{
			char *ext = strx(getExtWithDot(path));

			eraseExt(path);
			path = addLine(path, "~0");
			path = addLine(path, ext);

			memFree(ext);
		}

		path = toCreatablePath(path, faultCountMax);
	}
	return path;
}

uint c2i(int chr)
{
	return m_c2i(chr);
}
char *makeHexLine(autoBlock_t *block)
{
	char *line = (char *)memAlloc(getSize(block) * 2 + 1);
	uint index;

	for (index = 0; index < getSize(block); index++)
	{
		uint byte = getByte(block, index);

		line[index * 2 + 0] = hexadecimal[byte / 16];
		line[index * 2 + 1] = hexadecimal[byte % 16];
	}
	line[index * 2] = '\0';

	return line;
}
char *makeHexLine_x(autoBlock_t *block)
{
	char *out = makeHexLine(block);
	releaseAutoBlock(block);
	return out;
}
char *c_makeHexLine(autoBlock_t *block)
{
	static char *stock;
	memFree(stock);
	return stock = makeHexLine(block);
}
char *c_makeHexLine_x(autoBlock_t *block)
{
	static char *stock;
	memFree(stock);
	return stock = makeHexLine_x(block);
}
autoBlock_t *makeBlockHexLine(char *line)
{
	autoBlock_t *i = createBlock(strlen(line) / 2);
	char *p;
	uint byte;

	for (p = line; *p; )
	{
		byte = m_c2i(*p) & 0x0f;
		p++;

		errorCase(!*p);

		byte <<= 4;
		byte |= m_c2i(*p) & 0x0f;
		p++;

		addByte(i, byte);
	}
	return i;
}
autoBlock_t *makeBlockHexLine_x(char *line)
{
	autoBlock_t *out = makeBlockHexLine(line);
	memFree(line);
	return out;
}
autoBlock_t *c_makeBlockHexLine(char *line)
{
	static autoBlock_t *stock;
	if (stock) releaseAutoBlock(stock);
	return stock = makeBlockHexLine(line);
}
autoBlock_t *c_makeBlockHexLine_x(char *line)
{
	static autoBlock_t *stock;
	if (stock) releaseAutoBlock(stock);
	return stock = makeBlockHexLine_x(line);
}

void line2csym_ext(char *line, char *extra)
{
	static char *altchrs;
	char *p;
	int chr;

	errorCase(!*line); // "" 不可

	if (!altchrs)
	{
		altchrs = strx("");

		for (chr = '0'; chr <= '9'; chr++) { altchrs = addChar(altchrs, chr); }
		for (chr = 'A'; chr <= 'Z'; chr++) { altchrs = addChar(altchrs, chr); }
		for (chr = 'a'; chr <= 'z'; chr++) { altchrs = addChar(altchrs, chr); }

		altchrs = addLine(altchrs, "__");

		errorCase(strlen(altchrs) != 64); // 2bs
	}

	for (p = line; *p; p++)
	{
		chr = *p;

		if ('0' <= chr && chr <= '9' || 'A' <= chr && chr <= 'Z' || 'a' <= chr && chr <= 'z' || chr == '_' || extra && strchr(extra, chr))
		{
			// noop
		}
		else
		{
			*p = altchrs[chr & 0x3f];
		}
	}
}
void line2csym(char *line)
{
	line2csym_ext(line, NULL);
}
void line2fsym(char *line)
{
	autoList_t *tokens = tokenize(line, '.');
	char *token;
	uint index;
	char *newLine;

	trimLines(tokens);
	errorCase(!getCount(tokens)); // "" "." ".." "..." etc. 不可

	foreach (tokens, token, index)
		line2csym_ext(token, "-");

	newLine = untokenize(tokens, ".");
	releaseDim(tokens, 1);
	errorCase(strlen(line) < strlen(newLine));
	strcpy(line, newLine);
	memFree(newLine);
}
int toHalf(int chr)
{
	m_toHalf(chr);
	return chr;
}
char *toPrintLine(autoBlock_t *block, int insRet)
{
	autoBlock_t *lineBuff = createBlock(getSize(block));
	uint index;

	for (index = 0; index < getSize(block); index++)
	{
		int chr = getByte(block, index);
		int doInsRet;

		// ? (LF || CR + not LF) && 終端ではない。
		doInsRet = insRet && (chr == '\n' || chr == '\r' && refByte(block, index + 1) != '\n') && index + 1 < getSize(block);

		if (0x20 <= chr && chr <= 0x7e || 0xa1 <= chr && chr <= 0xdf)
		{
			// noop
		}
		else
		{
			chr = (chr & 0x1f) | 0xc0;
		}
		addByte(lineBuff, chr);

		if (doInsRet)
			addByte(lineBuff, '\n');
	}
	return unbindBlock2Line(lineBuff);
}
char *toPrintLine_x(autoBlock_t *block, int insRet)
{
	char *retLine = toPrintLine(block, insRet);

	releaseAutoBlock(block);
	return retLine;
}
char *lineToPrintLine(char *line, int insRet)
{
	autoBlock_t gab;
	return toPrintLine(gndBlockLineVar(line, gab), insRet);
}
char *lineToPrintLine_x(char *line, int insRet)
{
	char *retLine = lineToPrintLine(line, insRet);

	memFree(line);
	return retLine;
}
void line2JToken(char *token, int okJpn, int okBlank)
{
	line2JLine(token, okJpn, okBlank, okBlank, okBlank);
	ucTrim(token);
}
void line2JLine(char *line, int okJpn, int okRet, int okTab, int okSpc)
{
	char *p;

	for (p = line; *p; p++)
	{
//LOGPOS(); // test
//cout("%02x%02x\n", p[0], p[1]); // test
//		if (_ismbblead(p[0]) && _ismbbtrail(p[1]))
		if (isJChar(p[0] << 8 | p[1]))
		{
//LOGPOS(); // test
			if (!okJpn)
			{
//LOGPOS(); // test
				p[0] = (p[0] & 0x1f) | 0xc0;
				p[1] = (p[1] & 0x1f) | 0xc0;
			}
//LOGPOS(); // test
			p++;
		}
		else if (0xf0 <= p[0] && p[1] && okJpn) // 0xf000 - 0xffff -> 外字と見なす。本当の外字の範囲は F040 ～ F9FF らしい。
		{
			const char defMBChr[] = "外";

			cout("外字と見なします。(%02x%02x)\n", p[0], p[1]);

			p[0] = defMBChr[0];
			p[1] = defMBChr[1];
			p++;
		}
		else if (*p == '\r') // CR-LF または CR のみを LF と見なす。
		{
			if (p[1] == '\n') // LF
			{
				copyLine(p + 1, p + 2);
			}
			*p = '\n';
			goto found_lf;
		}
		else if (*p == '\n') // LF
		{
		found_lf:
			if (!okRet) { goto enc_char; }
		}
		else if (*p == '\t')
		{
			if (!okTab) { goto enc_char; }
		}
		else if (*p == '\x20')
		{
			if (!okSpc) { goto enc_char; }
		}
		else
		{
			int chr = *p;

			if (0x21 <= chr && chr <= 0x7e || 0xa1 <= chr && chr <= 0xdf)
			{
				// noop
			}
			else
			{
			enc_char:
				*p = (*p & 0x1f) | 0xc0;
			}
		}
	}
}
int isJToken(char *token, int okJpn, int okBlank)
{
	char *swrk = strx(token);
	int retval;

	line2JToken(swrk, okJpn, okBlank);
	retval = !strcmp(token, swrk);

	memFree(swrk);
	return retval;
}
int isJLine(char *line, int okJpn, int okRet, int okTab, int okSpc)
{
	char *swrk = strx(line);
	int retval;

	line2JLine(swrk, okJpn, okRet, okTab, okSpc);
	retval = !strcmp(line, swrk);

	memFree(swrk);
	return retval;
}
char *lineToJDoc(char *line, int okRet)
{
	autoList_t *lines;
	uint index;

	line = strx(line);
	line2JLine(line, 1, 1, 1, 1);
	lines = tokenize_x(line, '\n');

	foreach (lines, line, index)
	{
//		line = replaceLine(line, "　", " ", 0); // 全角SPC -> 半角SPC // 廃止
//		replaceChar(line, '\r', ' '); // CR は line2JLine() が何とかしてくれたはず。
//		replaceChar(line, '\t', ' ');
		line = replaceLine(line, "\t", "　　", 0); // HACK

//		line2JLine(line, 1, 0, 0, 1); // moved
		trimEdge(line, ' ');

		setElement(lines, index, (uint)line);
	}
	trimEdgeLines(lines);
	line = untokenize(lines, "\n");
	releaseDim(lines, 1);

	/*
	if (!*line)
		line = addChar(line, DEFCHR);
	*/

	if (!okRet)
		replaceChar(line, '\n', ' ');

	return line;
}
char *lineToJDoc_x(char *line, int okRet)
{
	char *retLine = lineToJDoc(line, okRet);

	memFree(line);
	return retLine;
}
char *lineToJDocMax(char *line, int okRet, uint lenmax)
{
	line = strx(line);
	setStrLenMax(line, lenmax);
	line = lineToJDoc_x(line, okRet);
	return line;
}
static autoList_t *GetWindowsReservedNodeList(void)
{
	static autoList_t *nodes;
	uint i;

	if (nodes)
		goto endfunc;

	nodes = newList();

	addElement(nodes, (uint)"AUX");
	addElement(nodes, (uint)"CON");
	addElement(nodes, (uint)"NUL");
	addElement(nodes, (uint)"PRN");

	for (i = 1; i <= 9; i++)
	{
		addElement(nodes, (uint)xcout("COM%u", i));
		addElement(nodes, (uint)xcout("LPT%u", i));
	}

	// グレーゾーン
	{
		addElement(nodes, (uint)"COM0");
		addElement(nodes, (uint)"LPT0");
		addElement(nodes, (uint)"CLOCK$");
		addElement(nodes, (uint)"CONFIG$");
	}

endfunc:
	return nodes;
}

#define L2FLP_NGCHRS "\"*/:<>?\\|"
#define L2FLP_DEFCHR '_'

/*
	使い方：
		lineToFairLocalPath(file, strlen( file が存在するディレクトリのフルパス ));   ... フルパスの長さを考慮する。
		lineToFairLocalPath(file, 0);                                                 ... フルパスの長さを考慮しない。それでも PATH_SIZE の制限は受ける。

	実際に作成可能なローカル名より基準が厳しい。
*/
char *lineToFairLocalPath(char *line, uint dirSize)
{
	autoList_t *winResNodes = GetWindowsReservedNodeList();
	autoList_t *nodes;
	char *node;
	uint index;
	char *p;
	uint lenmax = dirSize < PATH_SIZE ? PATH_SIZE - dirSize : 0;

	line = strx(line);

	if (lenmax < strlen(line))
	{
		line[lenmax] = '\0';
	}
	line2JLine(line, 1, 0, 0, 1);
	nodes = tokenize(line, '.');
	memFree(line);

	foreach (nodes, node, index)
	{
		trimEdge(node, ' ');

		if (!index && findLineCase(winResNodes, node, 1) < getCount(winResNodes))
		{
			*node = L2FLP_DEFCHR;
		}
		for (p = node; *p; p = mbsNext(p))
		{
			if (strchr(L2FLP_NGCHRS, *p))
			{
				*p = L2FLP_DEFCHR;
			}
		}
	}
	line = untokenize(nodes, ".");
	releaseDim(nodes, 1);

	if (!*line)
	{
		line = addChar(line, L2FLP_DEFCHR);
	}
	p = strchr(line, '\0') - 1;

	if (*p == '.')
	{
		*p = L2FLP_DEFCHR;
	}
	return line;
}
char *lineToFairLocalPath_x(char *line, uint dirSize)
{
	char *retLine = lineToFairLocalPath(line, dirSize);

	memFree(line);
	return retLine;
}
int isFairLocalPath(char *path, uint dirSize) // dirSize: path が存在するディレクトリのフルパスの長さ、考慮しない場合は 0 を指定すること。
{
	char *swrk;
	int retval;

	swrk = lineToFairLocalPath(path, dirSize);
	retval = !strcmp(path, swrk);

	memFree(swrk);
	return retval;
}
char *lineToFairRelPath(char *line, uint dirSize)
{
	autoList_t *tokens;
	char *token;
	uint index;

	line = strx(line);
	escapeYen(line);
	tokens = tokenize(line, '/');
	memFree(line);
	trimLines(tokens);

	foreach (tokens, token, index)
		setElement(tokens, index, (uint)lineToFairLocalPath_x(token, 0));

	line = untokenize(tokens, "\\");
	releaseDim(tokens, 1);

	if (!*line || PATH_SIZE < dirSize + strlen(line))
		line = lineToFairLocalPath_x(line, dirSize);

	return line;
}
char *lineToFairRelPath_x(char *line, uint dirSize)
{
	char *retLine = lineToFairRelPath(line, dirSize);

	memFree(line);
	return retLine;
}
int isFairRelPath(char *path, uint dirSize) // dirSize: path が存在するディレクトリのフルパスの長さ、考慮しない場合は 0 を指定すること。
{
	char *tmp;
	int retval;

	tmp = lineToFairRelPath(path, dirSize);
	retval = !strcmp(path, tmp);

	memFree(tmp);
	return retval;
}
int isFairHrefPath(char *path, int pathDelim) // pathDelim: "/\\"
{
	autoList_t *tokens;
	char *token;
	uint index;

	if (1000 < strlen(path)) // ? いくらなんでも長すぎる。
		return 0;

	if (pathDelim == '/')
	{
		tokens = tokenize(path, '/');
	}
	else if (pathDelim == '\\')
	{
		if (strchr(path, '/'))
			return 0;

		tokens = tokenizeYen(path);
	}
	else
	{
		error();
	}

	foreach (tokens, token, index)
		if (strcmp(token, ".") && strcmp(token, "..") && !isFairLocalPath(token, 0))
			break;

	releaseDim(tokens, 1);
	return !token;
}
char *toFairFullPathFltr(char *path) // path が不正な場合は error();
{
	autoList_t *ptkns;
	char *ptkn;
	uint ptknidx;
	int drvchr;
	char *result;

	errorCase(m_isEmpty(path));
	errorCase(strchr(path, '/')); // パスデリミタは '\\' のみ許可

	path = strx(path);

	if (lineExp("<1,AZaz>:<>", path)) // ? ドライブ名付き
	{
		drvchr = path[0];
		eraseLine(path, 2);

		if (!*path)
			path = addChar(path, '.'); // "" -> "."
	}
	else // ? ドライブ名無し
	{
		char *wkcwd = getCwd();

		drvchr = *wkcwd; // カレントドライブ
		memFree(wkcwd);
	}

	if (*path != '\\') // ? 相対パス
	{
		char *wkcwd;

		addCwd_x(xcout("%c:.", drvchr));
		wkcwd = getCwd(); // drvchr が { 存在しない || 準備できていない } -> ここで error();
		unaddCwd();

		if (!isAbsRootDir(wkcwd))
		{
			path = insertChar(path, 0, '\\');
			path = insertLine(path, 0, wkcwd + 3);
		}
		memFree(wkcwd);
	}
	else
	{
		eraseChar(path);
	}

	if (*path)
	{
		escapeYen(path);
		ptkns = tokenize(path, '/');
	}
	else
	{
		ptkns = newList();
	}

	for (ptknidx = 0; ptknidx < getCount(ptkns); )
	{
		ptkn = getLine(ptkns, ptknidx);

		if (!strcmp(ptkn, "."))
		{
			memFree((char *)desertElement(ptkns, ptknidx));
		}
		else if (!strcmp(ptkn, ".."))
		{
			errorCase(!ptknidx);

			ptknidx--;

			memFree((char *)desertElement(ptkns, ptknidx));
			memFree((char *)desertElement(ptkns, ptknidx));
		}
		else
		{
			ptknidx++;
		}
	}
	foreach (ptkns, ptkn, ptknidx)
	{
		if (!isFairLocalPath(ptkn, 0))
		{
			cout("Incorrected %u-th path token. \"%s\"\n", ptknidx + 1, lineToFairLocalPath(ptkn, 0));
			error();
		}
	}
	result = untokenize_xc(ptkns, "\\");
	result = insertLine_x(result, 0, xcout("%c:\\", drvchr));

	errorCase(PATH_SIZE < strlen(result));

	memFree(path);
	return result;
}
char *toFairFullPathFltr_x(char *path)
{
	char *retPath = toFairFullPathFltr(path);

	memFree(path);
	return retPath;
}

void autoIndent(autoList_t *lines)
{
	char *line;
	uint index;
	char *p;
	uint tabIndent;
	uint maxTabIndent;

	for (; ; )
	{
		foreach (lines, line, index)
			if (strchr(line, '\t'))
				break;

		if (!line)
			break;

		maxTabIndent = 0;

		foreach (lines, line, index)
		{
			p = strchr(line, '\t');

			if (p)
			{
				tabIndent = (uint)p - (uint)line;
				maxTabIndent = m_max(maxTabIndent, tabIndent);
			}
		}
		foreach (lines, line, index)
		{
			p = strchr(line, '\t');

			if (p)
			{
				*p = ' ';

				for (tabIndent = (uint)p - (uint)line; tabIndent < maxTabIndent; tabIndent++)
				{
					line = insertChar(line, tabIndent, ' ');
				}
				setElement(lines, index, (uint)line);
			}
		}
	}
}
void autoLeftIndent(autoList_t *lines, uint span)
{
	char *line;
	uint index;
	char *p;
	uint tabnum;
	uint insIndex;

	foreach (lines, line, index)
	{
		for (p = line; *p; p++)
			if (*p != '\t')
				break;

		tabnum = (uint)p - (uint)line;
		eraseLine(line, tabnum);
		line = insertLine_x(line, 0, repeatChar(' ', tabnum * span));
		setElement(lines, index, (uint)line);
	}
}

char *constrfltr(char *str)
{
	static autoList_t *strtbl;
	uint index;

	if (!strtbl)
		strtbl = newList();

	index = findLine(strtbl, str);

	if (index == getCount(strtbl))
		addElement(strtbl, (uint)strx(str));

	return getLine(strtbl, index);
}
char *constrfltr_x(char *str)
{
	char *out = constrfltr(str);
	memFree(str);
	return out;
}

char *getHexStr(void *block, uint size)
{
	autoBlock_t gab;
	return makeHexLine(gndBlockVar(block, size, gab));
}
char *c_getHexStr(void *block, uint size)
{
	static char *out;
	memFree(out);
	return out = getHexStr(block, size);
}

uint revBit(uint value, uint startBit, uint bitNum)
{
	uint l = startBit;
	uint h = startBit + bitNum - 1;
	uint ret = 0;

	while (l < h)
	{
		uint lb = value >> l & 1;
		uint hb = value >> h & 1;

		ret |= lb << h;
		ret |= hb << l;

		l++;
		h--;
	}
	return ret;
}

#define BASE64_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
#define BASE64_PAD '='

#define GetIndexOfBase64(chr) \
	( \
		'A' <= (chr) && (chr) <= 'Z' ? (chr) - 'A' : \
		'a' <= (chr) && (chr) <= 'z' ? (chr) - 'a' + 26 : \
		'0' <= (chr) && (chr) <= '9' ? (chr) - '0' + 52 : \
		(chr) == '+' ? 62 : 63 \
	)

autoBlock_t *encodeBase64(autoBlock_t *src)
{
	autoBlock_t *dest = newBlock();
	uint index;
	uint value;
	uint remaining;

	for (index = 0; index + 3 <= getSize(src); index += 3)
	{
		value =
			getByte(src, index + 0) << 16 |
			getByte(src, index + 1) <<  8 |
			getByte(src, index + 2) <<  0;

		addByte(dest, BASE64_CHARS[value >> 18 & 63]);
		addByte(dest, BASE64_CHARS[value >> 12 & 63]);
		addByte(dest, BASE64_CHARS[value >>  6 & 63]);
		addByte(dest, BASE64_CHARS[value >>  0 & 63]);
	}
	remaining = getSize(src) - index;

	if (remaining == 2)
	{
		value =
			getByte(src, index + 0) << 8 |
			getByte(src, index + 1) << 0;

		value = revBit(value, 0, 16);
		value =
			revBit(value,  0, 6) |
			revBit(value,  6, 6) |
			revBit(value, 12, 6);

		addByte(dest, BASE64_CHARS[value >>  0 & 63]);
		addByte(dest, BASE64_CHARS[value >>  6 & 63]);
		addByte(dest, BASE64_CHARS[value >> 12 & 63]);
		addByte(dest, BASE64_PAD);
	}
	else if (remaining == 1)
	{
		value = getByte(src, index);

		value = revBit(value, 0, 8);
		value =
			revBit(value, 0, 6) |
			revBit(value, 6, 6);

		addByte(dest, BASE64_CHARS[value >> 0 & 63]);
		addByte(dest, BASE64_CHARS[value >> 6 & 63]);
		addByte(dest, BASE64_PAD);
		addByte(dest, BASE64_PAD);
	}
	return dest;
}
autoBlock_t *decodeBase64(autoBlock_t *src)
{
	autoBlock_t *dest = newBlock();
	uint src_size = getSize(src);
	uint index;
	uint b1;
	uint b2;
	uint b3;
	uint b4;
	uint value;
	uint remaining;

	while (src_size && getByte(src, src_size - 1) == BASE64_PAD)
		src_size--;

	for (index = 0; index + 4 <= src_size; index += 4)
	{
		b1 = getByte(src, index + 0);
		b2 = getByte(src, index + 1);
		b3 = getByte(src, index + 2);
		b4 = getByte(src, index + 3);

		value =
			GetIndexOfBase64(b1) << 18 |
			GetIndexOfBase64(b2) << 12 |
			GetIndexOfBase64(b3) <<  6 |
			GetIndexOfBase64(b4) <<  0;

		addByte(dest, value >> 16 & 0xff);
		addByte(dest, value >>  8 & 0xff);
		addByte(dest, value >>  0 & 0xff);
	}
	remaining = src_size - index;

	if (remaining == 3)
	{
		b1 = getByte(src, index + 0);
		b2 = getByte(src, index + 1);
		b3 = getByte(src, index + 2);

		value =
			GetIndexOfBase64(b1) <<  0 |
			GetIndexOfBase64(b2) <<  6 |
			GetIndexOfBase64(b3) << 12;

		value =
			revBit(value,  0, 6) |
			revBit(value,  6, 6) |
			revBit(value, 12, 6);
		value =
			revBit(value, 0, 16);

		addByte(dest, value >> 8 & 0xff);
		addByte(dest, value >> 0 & 0xff);
	}
	else if (remaining == 2)
	{
		b1 = getByte(src, index + 0);
		b2 = getByte(src, index + 1);

		value =
			GetIndexOfBase64(b1) << 0 |
			GetIndexOfBase64(b2) << 6;

		value =
			revBit(value, 0, 6) |
			revBit(value, 6, 6);
		value =
			revBit(value, 0, 8);

		addByte(dest, value & 0xff);
	}
	return dest;
}

int isLine(char *line)
{
	return line && isJLine(line, 1, 0, 1, 1);
}
/*
	単一行だろうけど、絶対に単一行でないと困る文字列用フィルタ
	改行区切りの引数ファイルの行データに使用することを想定
	例：
		writeLine(optFp, asLine(option_01));
		writeLine(optFp, asLine(option_02));
		writeLine(optFp, asLine(option_03));

	行データに改行が存在すると、それ以降の引数が改行の個数分ずれてしまい、どのような作用をもたらすか想定しづらい。
	出力ファイル名がずれて意図しない文字列になってしまい、システムや重要なファイルを破壊するかもしれない。
*/
char *asLine(char *line)
{
	errorCase(!isLine(line));
	return line;
}
