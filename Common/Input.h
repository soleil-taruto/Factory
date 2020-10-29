extern int coil_esc;
extern int ignoreAllProcPoundKey;

void setAllProcPoundKey(int key);
int getKey(void);
void ungetKey(int key);
int hasKey(void);
void clearKey(void);
int clearGetKey(void);
int clearWaitKey(uint millis);
int clearCoWaitKey(uint millis);
int checkKey(int key);
int waitKey(uint millis);
int coWaitKey(uint millis);

autoList_t *editLines(autoList_t *lines);
autoList_t *editLines_x(autoList_t *lines);
char *editLine(char *line);
char *editLine_x(char *line);
autoList_t *inputLines(void);
char *inputLine(void);
char *coInputLine(void);
char *coInputLinePrn(void (*printFunc)(char *jbuffer));
autoBlock_t *inputTextAsBinary(void);
void viewLines(autoList_t *lines);
void viewLine(char *line);
void viewLineNoRet_NB(char *line, int nonBlockingMode);
void viewLineNoRet(char *line);

char *dropPath(void);
char *dropFile(void);
char *dropDir(void);
char *dropDirFile(void);

// c_
char *c_dropPath(void);
char *c_dropFile(void);
char *c_dropDir(void);
char *c_dropDirFile(void);
