#define m_isEmpty(line) \
	(!(line) || !*(line))

int isEmptyJTkn(char *token);

#define isMbc1(c) ( \
	'\x81' <= (c) && (c) <= '\x9f' || \
	'\xe0' <= (c) && (c) <= '\xfc')

#define isMbc(p) \
	(isMbc1((p)[0]) && (p)[1])
//	(_ismbblead((p)[0]) && (p)[1])
//	(_ismbblead((p)[0]) && _ismbbtrail((p)[1]))

#define mbsNext(p) \
	(p + (isMbc(p) ? 2 : 1))

#define m_isdecimal(c) ('0' <= (c) && (c) <= '9')
#define m_isbinadecimal(c) ((c) == '0' || (c) == '1')
#define m_isoctodecimal(c) ('0' <= (c) && (c) <= '7')
#define m_ishexadecimal(c) (m_isdecimal(c) || \
	'A' <= (c) && (c) <= 'F' || \
	'a' <= (c) && (c) <= 'f')

#define m_isalpha(c) (m_isupper(c) || m_islower(c))
#define m_isupper(c) ('A' <= (c) && (c) <= 'Z')
#define m_islower(c) ('a' <= (c) && (c) <= 'z')

#define m_isascii(c) ('\x20' <= (c) && (c) <= '\x7e')
#define m_iskana(c)  ('\xa1' <= (c) && (c) <= '\xdf')
#define m_isasciikana(c) (m_isascii(c) || m_iskana(c)) // m_isHalf @ DataConv.h ‚Æ“¯‚¶

#define m_nctoupper(c) ((c) - 0x20)
#define m_nctolower(c) ((c) + 0x20)
#define m_toupper(c) (m_islower((c)) ? m_nctoupper(c) : (c))
#define m_tolower(c) (m_isupper((c)) ? m_nctolower(c) : (c))

#define m_chricmp(c, d) ((sint)m_tolower((c)) - (sint)m_tolower((d)))

#define m_isspace(c) ((c) <= ' ')
#define m_ispunct(c) ( \
	'!' <= (c) && (c) <= '/' || \
	':' <= (c) && (c) <= '@' || \
	'[' <= (c) && (c) <= '`' || \
	'{' <= (c) && (c) <= '~')

#define m_iscsymf(c) ((c) == '_' || m_isalpha(c))
#define m_iscsym(c)  ((c) == '_' || m_isalpha(c) || m_isdecimal(c))

int c2upper(int c);
int c2lower(int c);

sint chricmp(int c, int d);

char *strxm(char *line, uint buffsize);
char *strrm(char *line, uint buffsize);

char *strx(char *line);
char *strr(char *line);
char *strxl(char *line, uint count);
char *strz(char *buffer, char *line);
void strzp(char **p_buffer, char *line);

char *strxRng(char *p, char *q);

void replaceChar(char *line, int findChr, int repChr);
void unizChar(char *line, char *findChrs, int repChr);
void escapeYen(char *path);
void restoreYen(char *path);
autoList_t *tokenizeYen_heap(char *path);
autoList_t *tokenizeYen(char *path);
autoList_t *tokenizeYen_x(char *path);
sint mbs_strnicmp(char *line1, char *line2, uint count);
sint mbs_stricmp(char *line1, char *line2);
char *mbs_strchr(char *line, int findChr);
char *mbs_strrchr(char *line, int findChr);
char *strchrEnd(char *line, int findChr);
char *strchrNext(char *line, int findChr);
char *ne_strchr(char *line, int findChr);
char *ne_strstr(char *line, char *findPtn);
char *nn_strchr(char *line, int findChr);
char *nn_strstr(char *line, char *findPtn);
uint strlen_max(char *str, uint retmax);
sint getNumStrSign(char *str);
sint numstrcmp(char *str1, char *str2);

int mbsStartsWithICase(char *line1, char *line2);
int startsWithICase(char *line1, char *line2);
int startsWith(char *line1, char *line2);

int endsWithICase(char *line1, char *line2);
int endsWith(char *line1, char *line2);

char *mbs_strstrCase(char *line, char *ptn, int ignoreCase);
char *mbs_strstr(char *line, char *ptn);
char *mbs_stristr(char *line, char *ptn);
char *strstrNextCase(char *line, char *ptn, int ignoreCase);
char *strstrNext(char *line, char *ptn);
char *stristrNext(char *line, char *ptn);
char *strstrEndCase(char *line, char *ptn, int ignoreCase);
char *strstrEnd(char *line, char *ptn);
char *stristrEnd(char *line, char *ptn);
char *strrstrCase(char *line, char *ptn, int ignoreCase);
char *strrstr(char *line, char *ptn);
char *strristr(char *line, char *ptn);
void replacePtn(char *line, char *ptn1, char *ptn2, int ignoreCase);
char *replaceLine(char *line, char *ptn1, char *ptn2, int ignoreCase);
char *replaceLineLoop(char *line, char *ptn1, char *ptn2, int ignoreCase, uint loopMax);
uint replaceLine_getLastReplacedCount(void);

void copyBlock(void *dest, void *src, uint size);
void copyLine(char *dest, char *src);
void removeChar(char *line, int chr);
void reverseLine(char *line);
char *thousandComma(char *line);

void trimLead(char *line, int delimChr);
void trimTrail(char *line, int delimChr);
void trimSequ(char *line, int delimChr);
void trim(char *line, int delimChr);
void trimEdge(char *line, int delimChr);

char *addLine(char *line, char *addPtn);
char *addChar(char *line, int chr);
void eraseLine(char *line, uint count);
void eraseChar(char *line);
char *insertLine(char *line, uint index, char *insPtn);
char *insertChar(char *line, uint index, int chr);

void toLowerLine(char *line);
void toUpperLine(char *line);

char *repeatChar(int chr, uint num);
char *charSetChar(char *chrs);
char *kanjiPunch(char *str, int knjChr);
char *setStrLenRng(char *str, uint lenmin, uint lenmax, int defchr);
char *setStrLenMin(char *str, uint lenmin, int defchr);
void setStrLenMax(char *str, uint lenmax);
void toAsciiLine(char *str, int okRet, int okTab, int okSpc);
int isAsciiLine(char *str, int okRet, int okTab, int okSpc);

void tokinit(char *str, char *delims);
char *toknext(char *str, char *delims);

// c_
char *c_thousandComma(char *line);

// _x
char *strz_x(char *buffer, char *line);
void strzp_x(char **p_buffer, char *line);
char *addLine_x(char *line, char *lineAdd);
uint strlen_x(char *line);
char *insertLine_x(char *line, uint index, char *insPtn);
char *replaceLine_cx(char *line, char *ptn1, char *ptn2, int ignoreCase);
char *replaceLine_xc(char *line, char *ptn1, char *ptn2, int ignoreCase);
