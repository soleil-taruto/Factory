char *getLine(autoList_t *lines, uint index);
char *refLine(autoList_t *lines, uint index);
char *zSetLine(autoList_t *lines, uint index, char *line);

autoList_t *copyLines(autoList_t *lines);
void addLines(autoList_t *lines, autoList_t *subLines);

uint findLineComp(autoList_t *lines, char *lineFind, sint (*funcComp)(char *, char *));
uint findLine(autoList_t *lines, char *lineFind);
uint findLineCase(autoList_t *lines, char *lineFind, uint ignoreCase);
uint findJLineICase(autoList_t *lines, char *lineFind);

autoList_t *tokenizeMinMax(char *line, int delimiter, uint nummin, uint nummax, char *def_token);
autoList_t *tokenize(char *line, int delimiter);
autoList_t *tokenizeDs(char *line, char *delims);
char *untokenize(autoList_t *tokens, char *separator);

void sortJLinesICase(autoList_t *lines);
void trimLines(autoList_t *lines);
void trimEdgeLines(autoList_t *lines);
void emptizeSubLines(autoList_t *lines, uint start, uint count);
void emptizeFollowLines(autoList_t *lines, uint start);
void rmtrimSubLines(autoList_t *lines, uint start, uint count);
void rmtrimFollowLines(autoList_t *lines, uint start);

void shootingStarLines_CSP(autoList_t *lines, uint count, int starChr, int padChr);
void shootingStarLines(autoList_t *lines);
void spacingStarLines_SP(autoList_t *lines, uint lenmin, int starChr, int padChr);
void spacingStarLines(autoList_t *lines, uint lenmin);

// _x
void addLines_x(autoList_t *lines, autoList_t *subLines);
autoList_t *tokenize_x(char *line, int delimiter);
char *untokenize_cx(autoList_t *tokens, char *separator);
char *untokenize_xc(autoList_t *tokens, char *separator);
char *untokenize_xx(autoList_t *tokens, char *separator);
