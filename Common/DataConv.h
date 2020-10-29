char *incrementColumnDigits(char *column, char *digits);
char *incrementPathDigits(char *path, char *digits);
char *incrementColumn(char *column);
char *incrementPath(char *path);

char *toCreatablePath(char *path, uint faultCountMax);
char *toCreatableTildaPath(char *path, uint faultCountMax);

#define m_c2i(chr) \
	((chr) & 0x40 ? ((chr) & 0x1f) + 9 : (chr) & 0x0f)

#define m_i2c(val) \
	((val) < 10 ? '0' + (val) : 'a' + (val) - 10)

uint c2i(int chr);
char *makeHexLine(autoBlock_t *block);
char *makeHexLine_x(autoBlock_t *block);
char *c_makeHexLine(autoBlock_t *block);
char *c_makeHexLine_x(autoBlock_t *block);
autoBlock_t *makeBlockHexLine(char *line);
autoBlock_t *makeBlockHexLine_x(char *line);
autoBlock_t *c_makeBlockHexLine(char *line);
autoBlock_t *c_makeBlockHexLine_x(char *line);

/*
	パスの長さのおよその上限 -> Factory での上限とする。
	FAT32 は 255 バイト, NTFS は 255 文字が上限 -> それをちょっと短くしたあたり。
	NTFS は全角文字のみ使えば 511 バイト程度いけるが考慮しない。
*/
#define PATH_SIZE 250

void line2csym_ext(char *line, char *extra);
void line2csym(char *line);
void line2fsym(char *line);

// m_isasciikana @ Line.h と同じ
#define m_isHalf(chr) \
	(0x20 <= (chr) && (chr) <= 0x7e || 0xa1 <= (chr) && (chr) <= 0xdf)

#define m_toHalf(chr) \
	(m_isHalf((chr)) || ((chr) = (chr) & 0x1f | 0xc0))

int toHalf(int chr);
char *toPrintLine(autoBlock_t *block, int insRet);
char *toPrintLine_x(autoBlock_t *block, int insRet);
char *lineToPrintLine(char *line, int insRet);
char *lineToPrintLine_x(char *line, int insRet);
void line2JLine(char *line, int okJpn, int okRet, int okTab, int okSpc);
void line2JToken(char *token, int okJpn, int okBlank);
int isJLine(char *line, int okJpn, int okRet, int okTab, int okSpc);
int isJToken(char *token, int okJpn, int okBlank);
char *lineToJDoc(char *line, int okRet);
char *lineToJDoc_x(char *line, int okRet);
char *lineToJDocMax(char *line, int okRet, uint lenmax);
char *lineToFairLocalPath(char *line, uint dirSize);
char *lineToFairLocalPath_x(char *line, uint dirSize);
int isFairLocalPath(char *path, uint dirSize);
char *lineToFairRelPath(char *line, uint dirSize);
char *lineToFairRelPath_x(char *line, uint dirSize);
int isFairRelPath(char *path, uint dirSize);
int isFairHrefPath(char *path, int pathDelim);
char *toFairFullPathFltr(char *path);
char *toFairFullPathFltr_x(char *path);

void autoIndent(autoList_t *lines);
void autoLeftIndent(autoList_t *lines, uint span);

char *constrfltr(char *str);
char *constrfltr_x(char *str);

char *getHexStr(void *block, uint size);
char *c_getHexStr(void *block, uint size);

uint revBit(uint value, uint startBit, uint bitNum);

#define m_isBase64Char(chr) ( \
	'A' <= (chr) && (chr) <= 'Z' || \
	'a' <= (chr) && (chr) <= 'z' || \
	'0' <= (chr) && (chr) <= '9' || \
	(chr) == '+' || \
	(chr) == '/' )

autoBlock_t *encodeBase64(autoBlock_t *src);
autoBlock_t *decodeBase64(autoBlock_t *src);

int isLine(char *line);
char *asLine(char *line);
