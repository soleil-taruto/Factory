#include "all.h"

int isEmptyJTkn(char *token)
{
	if (m_isEmpty(token))
		return 1;

	if (!isJLine(token, 1, 0, 0, 1))
		return 1;

	if (token[0] == ' ')
		return 1;

	if (strchr(token, '\0')[-1] == ' ')
		return 1;

	return 0;
}

int c2upper(int c)
{
	return m_toupper(c);
}
int c2lower(int c)
{
	return m_tolower(c);
}

sint chricmp(int c, int d)
{
	return m_chricmp(c, d);
}

char *strxm(char *line, uint buffsize)
{
	return (char *)memCloneWithBuff(line, strlen(line) + 1, buffsize);
}
char *strrm(char *line, uint buffsize)
{
	return (char *)memRealloc(line, strlen(line) + 1 + buffsize);
}

char *strx(char *line)
{
	return strxm(line, 0);
}
char *strr(char *line)
{
	return strrm(line, 0);
}
char *strxl(char *line, uint count)
{
	errorCase(strlen_max(line, count) < count);

	line = memCloneWithBuff(line, count, 1);
	line[count] = '\0';

	return line;
}
char *strz(char *buffer, char *line)
{
	if (!buffer)
		buffer = (char *)memAlloc(1);

	buffer[0] = '\0';
	return addLine(buffer, line);
}
void strzp(char **p_buffer, char *line)
{
	*p_buffer = strz(*p_buffer, line);
}

char *strxRng(char *p, char *q)
{
	errorCase(!p);
	errorCase(!q);
	errorCase(q < p);

	return strxl(p, (uint)q - (uint)p);
}

void replaceChar(char *line, int findChr, int repChr) // mbs_
{
	char *p;

	for (p = line; *p; p = mbsNext(p))
	{
		if (*p == findChr)
		{
			*p = repChr;
		}
	}
}
void unizChar(char *line, char *findChrs, int repChr) // mbs_
{
	char *p;

	for (p = findChrs; *p; p++)
	{
		replaceChar(line, *p, repChr);
	}
}
void escapeYen(char *path)
{
	replaceChar(path, '\\', '/');
}
void restoreYen(char *path)
{
	replaceChar(path, '/', '\\');
}
autoList_t *tokenizeYen_heap(char *path) // path: ヒープ上のメモリブロックと見なして、内容を一時的に書き換える。
{
	autoList_t *ptkns;

	escapeYen(path);
	ptkns = tokenize(path, '/');
	restoreYen(path);
	return ptkns;
}
autoList_t *tokenizeYen(char *path)
{
	autoList_t *ptkns;

	path = strx(path);
	escapeYen(path);
	ptkns = tokenize(path, '/');
	memFree(path);
	return ptkns;
}
autoList_t *tokenizeYen_x(char *path)
{
	autoList_t *ret = tokenizeYen_heap(path); // 開放して良いならヒープ上のメモリブロックであるはず
	memFree(path);
	return ret;
}
/*
	strlen(line1) < count || strlen(line2) < count の時は _stricmp(line1, line2) と同じ
	count == 0 の時は、常に一致する。
*/
sint mbs_strnicmp(char *line1, char *line2, uint count)
{
	uint index;

	for (index = 0; index < count; index++)
	{
		if (line1[index] == '\0')
		{
			goto founddiff;
		}
		if (isMbc(line1 + index))
		{
			if (line1[index] != line2[index]) goto founddiff;
			index++;
			if (line1[index] != line2[index]) goto founddiff;
		}
		else
		{
			int chr1 = line1[index];
			int chr2 = line2[index];

			chr1 = m_tolower(chr1);
			chr2 = m_tolower(chr2);

			if (chr1 != chr2)
			{
				return chr1 - chr2;
			}
		}
	}
	return 0;

founddiff:
	return (sint)line1[index] - (sint)line2[index];
}
sint mbs_stricmp(char *line1, char *line2)
{
	return mbs_strnicmp(line1, line2, UINTMAX);
}
char *mbs_strchr(char *line, int findChr)
{
	char *p;

	for (p = line; *p; p = mbsNext(p))
	{
		if (*p == findChr)
		{
			return p;
		}
	}
	return NULL;
}
char *mbs_strrchr(char *line, int findChr) // _mbsrchr() と同じ関数ではないか？
{
	char *foundPtr = NULL;
	char *p;

	for (p = line; *p; p = mbsNext(p))
	{
		if (*p == findChr)
		{
			foundPtr = p;
		}
	}
	return foundPtr;
}
char *strchrEnd(char *line, int findChr)
{
	char *p = strchr(line, findChr);

	if (!p)
		p = strchr(line, '\0');

	errorCase(!p);
	return p;
}
char *strchrNext(char *line, int findChr)
{
	char *p;

	errorCase(findChr == '\0');

	p = strchr(line, findChr);

	if (p)
		p++;
	else
		p = strchr(line, '\0');

	return p;
}
char *ne_strchr(char *line, int findChr)
{
	char *ret = strchr(line, findChr);

	errorCase(!ret);
	return ret;
}
char *ne_strstr(char *line, char *findPtn)
{
	char *ret = strstr(line, findPtn);

	errorCase(!ret);
	return ret;
}
char *nn_strchr(char *line, int findChr)
{
	return strchrEnd(line, findChr);
}
char *nn_strstr(char *line, char *findPtn)
{
	return strstrEnd(line, findPtn);
}
uint strlen_max(char *str, uint retmax)
{
	uint count;

	for (count = 0; str[count] && count < retmax; count++);

	return count;
}
sint getNumStrSign(char *str)
{
	return strchr(str, '-') ? -1 : 1;
}
sint numstrcmp(char *str1, char *str2)
{
	sint ret = getNumStrSign(str1) - getNumStrSign(str2);

	if (ret)
		return ret;

	ret = (sint)strlen(str1) - (sint)strlen(str2); // 桁数が多い方は文字列として長いはず。

	if (ret)
		return ret;

	ret = strcmp(str1, str2);
	return ret;
}

/*
	ret: line1 は line2 で始まるか判定する。
		strlen(line1) < strlen(line2) の時は常に異なる。(0を返す)
		line2 == "" の時は常に一致する。(0以外を返す)
*/
int mbsStartsWithICase(char *line1, char *line2)
{
	return !mbs_strnicmp(line1, line2, strlen(line2));
}
int startsWithICase(char *line1, char *line2)
{
	return !_strnicmp(line1, line2, strlen(line2));
}
int startsWith(char *line1, char *line2)
{
	return !strncmp(line1, line2, strlen(line2));
}

/*
	ret: line1 は line2 で終わるか判定する。
		strlen(line1) < strlen(line2) の時は常に異なる。(0を返す)
		line2 == "" の時は常に一致する。(0以外を返す)
*/
static int Prv_EndsWith(char *line1, char *line2, int (*func_strcmp)(char *, char *))
{
	uint len1 = strlen(line1);
	uint len2 = strlen(line2);

	if (len1 < len2)
		return 0;

	return !func_strcmp(line1 + len1 - len2, line2);
}
int endsWithICase(char *line1, char *line2)
{
	return Prv_EndsWith(line1, line2, _stricmp);
}
int endsWith(char *line1, char *line2)
{
	return Prv_EndsWith(line1, line2, strcmp);
}

/*
	ptn == "" のときは line を返す。<- strstr() と同じ挙動
*/
char *mbs_strstrCase(char *line, char *ptn, int ignoreCase)
{
	uint ptnlen = strlen(ptn);

	if (ptnlen <= strlen(line))
	{
		char *end = strchr(line, '\0') - ptnlen;
		char *p;

		for (p = line; p <= end; p = mbsNext(p))
		{
			if ((ignoreCase ? mbs_strnicmp : strncmp)(p, ptn, ptnlen) == 0)
			{
				return p;
			}
		}
	}
	return NULL;
}
char *mbs_strstr(char *line, char *ptn)
{
	return mbs_strstrCase(line, ptn, 0);
}
char *mbs_stristr(char *line, char *ptn)
{
	return mbs_strstrCase(line, ptn, 1);
}

char *strstrNextCase(char *line, char *ptn, int ignoreCase) // mbs_
{
	char *p = mbs_strstrCase(line, ptn, ignoreCase);

	if (p)
		p += strlen(ptn);

	return p;
}
char *strstrNext(char *line, char *ptn) // mbs_
{
	return strstrNextCase(line, ptn, 0);
}
char *stristrNext(char *line, char *ptn) // mbs_
{
	return strstrNextCase(line, ptn, 1);
}
char *strstrEndCase(char *line, char *ptn, int ignoreCase) // mbs_
{
	char *p = mbs_strstrCase(line, ptn, ignoreCase);

	if (!p)
		p = strchr(line, '\0');

	errorCase(!p);
	return p;
}
char *strstrEnd(char *line, char *ptn) // mbs_
{
	return strstrEndCase(line, ptn, 0);
}
char *stristrEnd(char *line, char *ptn) // mbs_
{
	return strstrEndCase(line, ptn, 1);
}

char *strrstrCase(char *line, char *ptn, int ignoreCase) // mbs_
{
	char *ret = NULL;

	for (; ; )
	{
		char *p = mbs_strstrCase(line, ptn, ignoreCase);

		if (!p)
			break;

		ret = p;
		line = mbsNext(p);      // strrstr("ABABA", "ABA"); のとき "ABABA" + 2 を返す。
//		line = p + strlen(ptn); // strrstr("ABABA", "ABA"); のとき "ABABA" + 0 を返す。
	}
	return ret;
}
char *strrstr(char *line, char *ptn) // mbs_
{
	return strrstrCase(line, ptn, 0);
}
char *strristr(char *line, char *ptn) // mbs_
{
	return strrstrCase(line, ptn, 1);
}

void replacePtn(char *line, char *ptn1, char *ptn2, int ignoreCase) // mbs_
{
	char *p = line;
	uint ptnSz = strlen(ptn1);

	errorCase(ptnSz != strlen(ptn2));

	while (p = mbs_strstrCase(p, ptn1, ignoreCase))
	{
		memcpy(p, ptn2, ptnSz);
		p += ptnSz;
	}
}

static uint LastReplacedCount;

char *replaceLine(char *line, char *ptn1, char *ptn2, int ignoreCase) // mbs_ ret: strr(line)
{
	uint si = 0;
	char *p;

	errorCase(*ptn1 == '\0'); // ? ptn1 == ""

	LastReplacedCount = 0;

	while (p = mbs_strstrCase(line + si, ptn1, ignoreCase))
	{
		char *lold = line;

		line = strxl(line, (uint)p - (uint)line);
		line = addLine(line, ptn2);
		si = strlen(line);
		line = addLine(line, p + strlen(ptn1));

		memFree(lold);

		LastReplacedCount++;
	}
	return line;
}
char *replaceLineLoop(char *line, char *ptn1, char *ptn2, int ignoreCase, uint loopMax) // mbs_ ret: strr(line)
{
	while (loopMax)
	{
		line = replaceLine(line, ptn1, ptn2, ignoreCase);

		if (!LastReplacedCount)
			break;

		loopMax--;
	}
	return line;
}
uint replaceLine_getLastReplacedCount(void)
{
	return LastReplacedCount;
}

//#define CB_MINSIZE 16 // めっちゃ重なってる時、遅い。@ 2017.5.15
#define CB_MINSIZE 1024

static uchar CB_Buff[CB_MINSIZE];

void copyBlock(void *dest, void *src, uint size) // memcpy() と異なり、領域の重複を許可する。
{
	if (dest == src)
		return;

	if (!size) // memcpy(,,0) しても大丈夫だと思うけど、念のため回避
		return;

	if ((uchar *)dest + size <= src || (uchar *)src + size <= dest) // ? 重複ナシ
	{
		memcpy(dest, src, size);
	}
	else if (size <= CB_MINSIZE)
	{
		memcpy(CB_Buff, src, size);
		memcpy(dest, CB_Buff, size);
	}
	else
	{
		uint half = size / 2;

		if (dest < src)
			copyBlock(dest, src, half);

		copyBlock((uchar *)dest + half, (uchar *)src + half, size - half);

		if (src < dest)
			copyBlock(dest, src, half);
	}
}
void copyLine(char *dest, char *src) // strcpy() と異なり、領域の重複を許可する。
{
	copyBlock(dest, src, strlen(src) + 1);
}
void removeChar(char *line, int chr)
{
	char *n = strchr(line, chr);

	if (n)
	{
		char *f;

		errorCase(!*n); // ? !chr

		for (f = n + 1; *f; f++)
		{
			if (*f != chr)
			{
				*n++ = *f;
			}
		}
		*n = '\0';
	}
}
void reverseLine(char *line)
{
	char *lnear = line;
	char *lfar = strchr(line, '\0');

	if (lnear < lfar)
	{
		lfar--;

		while (lnear < lfar)
		{
			int swap = lnear[0];

			lnear[0] = lfar[0];
			lfar[0] = swap;

			lnear++; lfar--;
		}
	}
}
char *thousandComma(char *line) // ret: strr(line)
{
	uint index;

	reverseLine(line); // 逆転

	for (index = 3; index < strlen(line); index += 4)
	{
		line = insertChar(line, index, ',');
	}
	reverseLine(line); // 復元
	return line;
}

void trimLead(char *line, int delimChr)
{
	char *p;

	for (p = line; *p; p++)
	{
		if (*p != delimChr)
		{
			break;
		}
	}
	copyLine(line, p);
}
void trimTrail(char *line, int delimChr)
{
	char *p;

	for (p = strchr(line, '\0'); line < p; p--)
	{
		if (p[-1] != delimChr)
		{
			break;
		}
	}
	*p = '\0';
}
void trimSequ(char *line, int delimChr)
{
	char *n = line;

	errorCase(delimChr == '\0');

	while (n = strchr(n, delimChr))
	{
		n++;

		if (*n == delimChr)
		{
			char *f = n;

			do
			{
				f++;
			}
			while (*f == delimChr);

			while (*f)
			{
				if (*f == delimChr)
					while (f[1] == delimChr)
						f++;

				*n++ = *f++;
			}
			*n = '\0';
			break;
		}
	}
}
void trim(char *line, int delimChr)
{
	trimTrail(line, delimChr);
	trimLead(line, delimChr);
	trimSequ(line, delimChr);
}
void trimEdge(char *line, int delimChr)
{
	trimTrail(line, delimChr);
	trimLead(line, delimChr);
}

char *addLine(char *line, char *addPtn)
{
	line = strrm(line, strlen(addPtn));
	strcat(line, addPtn);
	return line;
}
char *addChar(char *line, int chr)
{
	char chrLine[2];

	chrLine[0] = chr;
	chrLine[1] = '\0';

	return addLine(line, chrLine);
}
void eraseLine(char *line, uint count)
{
	errorCase(strlen(line) < count);
	copyLine(line, line + count);
}
// memo:
// 文字列から特定の文字を削除する。--> removeChar
// 文字列から空白とコントロール文字を削除する。--> removeBlank
void eraseChar(char *line)
{
	eraseLine(line, 1);
}
char *insertLine(char *line, uint index, char *insPtn)
{
	char *trailer;

	errorCase(strlen(line) < index);

	trailer = strx(line + index);

	line = strrm(line, strlen(insPtn));
	strcpy(line + index, insPtn);
	strcat(line, trailer);

	memFree(trailer);
	return line;
}
char *insertChar(char *line, uint index, int chr)
{
	char insPtn[2];

	insPtn[0] = chr;
	insPtn[1] = '\0';

	return insertLine(line, index, insPtn);
}

void toLowerLine(char *line) // mbs_
{
	char *p;

	for (p = line; *p; p = mbsNext(p))
	{
		*p = m_tolower(*p);
	}
}
void toUpperLine(char *line) // mbs_
{
	char *p;

	for (p = line; *p; p = mbsNext(p))
	{
		*p = m_toupper(*p);
	}
}

char *repeatChar(int chr, uint num)
{
	char *line = (char *)memAlloc(num + 1);

	memset(line, chr, num);
	line[num] = '\0';
	return line;
}
char *charSetChar(char *chrs)
{
	char *line = strx("");
	char *p;
	int minchr;
	int maxchr;
	int c;

	for (p = chrs; *p; p++)
	{
		minchr = *p;
		p++;
		maxchr = *p;

		for (c = minchr; c <= maxchr; c++)
		{
			line = addChar(line, c);
		}
	}
	return line;
}
char *kanjiPunch(char *str, int knjChr)
{
	char *p;

	str = strx(str);

	for (p = str; *p; p++)
	{
		if (isMbc(p))
		{
			*p++ = knjChr;
			*p = knjChr;
		}
	}
	return str;
}
char *setStrLenRng(char *str, uint lenmin, uint lenmax, int defchr) // ret: strlen(str) < lenmin ? strr(str) : str
{
	str = setStrLenMin(str, lenmin, defchr);
	setStrLenMax(str, lenmax);
	return str;
}
char *setStrLenMin(char *str, uint lenmin, int defchr) // ret: strlen(str) < lenmin ? strr(str) : str
{
	int count = strlen(str);

	while (count < lenmin)
	{
		str = addChar(str, defchr);
		count++;
	}
	return str;
}
void setStrLenMax(char *str, uint lenmax)
{
#if 1
	str[strlen_max(str, lenmax)] = '\0';
#else // old_almost_same
	if (lenmax < strlen(str))
		str[lenmax] = '\0';
#endif
}
void toAsciiLine(char *str, int okRet, int okTab, int okSpc)
{
	char *p;

	for (p = str; *p; p++)
	{
		if (*p == '\r') // CR-LF 又は CR を LF にする。
		{
			if (p[1] == '\n')
				copyLine(p, p + 1);
			else
				*p = '\n';

			goto found_lf;
		}
		else if (*p == '\n')
		{
		found_lf:
			if (!okRet) { goto enc_char; }
		}
		else if (*p == '\t')
		{
			if (!okTab) { goto enc_char; }
		}
		else if (*p == ' ')
		{
			if (!okSpc) { goto enc_char; }
		}
		else if (m_isRange(*p, '\x21', '\x7e'))
		{
			// noop
		}
		else
		{
		enc_char:
			*p = (*p & 0x1f) | 0x40;
		}
	}
}
int isAsciiLine(char *str, int okRet, int okTab, int okSpc)
{
	char *tmp = strx(str);
	int ret;

	toAsciiLine(tmp, okRet, okTab, okSpc);
	ret = !strcmp(tmp, str);
	memFree(tmp);
	return ret;
}

static char *TokPtr;
static uchar *TokDelims;

void tokinit(char *str, char *delims)
{
	if (str)
	{
		TokPtr = str;
		memFree(TokDelims);
		TokDelims = NULL;
	}
	if (delims)
	{
		char *p;

		if (*delims)
		{
			if (!TokDelims)
				TokDelims = memAlloc(32);

			memset(TokDelims, 0x00, 32);

			for (p = delims; ; p++)
			{
				uint delim = *p;

				TokDelims[delim / 8] |= 1 << delim % 8;

				if (!delim)
					break;
			}
		}
		else
		{
			memFree(TokDelims);
			TokDelims = NULL;
		}
	}
}
char *toknext(char *str, char *delims)
{
	char *ret;

	tokinit(str, delims);

	if (!TokPtr)
		return NULL;

	ret = TokPtr;

	if (TokDelims)
	{
		char *p;
		uint upchr;

		for (p = TokPtr; ; p++)
		{
			upchr = *p;

			if (TokDelims[upchr / 8] & 1 << upchr % 8)
				break;
		}
		if (upchr)
		{
			*p = '\0';
			p++;
		}
		else
		{
			p = NULL;
		}
		TokPtr = p;
	}
	else
	{
		TokPtr = NULL;
	}
	return ret;
}

// c_
char *c_thousandComma(char *line)
{
	static char *stock;
	memFree(stock);
	return stock = thousandComma(line);
}

// _x
char *strz_x(char *buffer, char *line)
{
#if 1
	memFree(buffer);
	return line;
#else // SAME CODE
	char *out = strz(buffer, line);
	memFree(line);
	return out;
#endif
}
void strzp_x(char **p_buffer, char *line)
{
	strzp(p_buffer, line);
	memFree(line);
}
char *addLine_x(char *line, char *lineAdd)
{
	char *out = addLine(line, lineAdd);
	memFree(lineAdd);
	return out;
}
uint strlen_x(char *line)
{
	uint retval = strlen(line);
	memFree(line);
	return retval;
}
char *insertLine_x(char *line, uint index, char *insPtn)
{
	char *out = insertLine(line, index, insPtn);
	memFree(insPtn);
	return out;
}
char *replaceLine_cx(char *line, char *ptn1, char *ptn2, int ignoreCase)
{
	char *out = replaceLine(line, ptn1, ptn2, ignoreCase);
	memFree(ptn2);
	return out;
}
char *replaceLine_xc(char *line, char *ptn1, char *ptn2, int ignoreCase)
{
	char *out = replaceLine(line, ptn1, ptn2, ignoreCase);
	memFree(ptn1);
	return out;
}
