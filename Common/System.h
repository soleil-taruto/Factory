// ---- process properties ----

extern int sockServerMode;
extern int noErrorDlgMode;
extern uint sockConnectTimeoutSec;
extern char *majorOutputLinePrefix;

// ----

#define SLEEP_NICK_MILLIS 250

extern int lastSystemRet;

void execute(char *commandLine);
void execute_x(char *commandLine);
void coExecute(char *commandLine);
void coExecute_x(char *commandLine);
void cmdTitle(char *title);
void cmdTitle_x(char *title);
void sleep(uint millis);
void coSleep(uint millis);
void noop(void);
void noop_u(uint dummy);
void noop_uu(uint dummy1, uint dummy2);
uint echo_u(uint prm);
uint getZero(void);
uint getZero_u(uint dummy);
uint getZero_uu(uint dummy1, uint dummy2);
char *getEnvLine(char *name);
uint64 nowTick(void);
uint now(void);
int pulseSec(uint span, uint *p_nextSec);
int eqIntPulseSec(uint span, uint *p_nextSec);
//uint getTick(void);
uint getUDiff(uint tick1, uint tick2);
sint getIDiff(uint tick1, uint tick2);
uint64 getUDiff64(uint64 tick1, uint64 tick2);
sint64 getIDiff64(uint64 tick1, uint64 tick2);
int isLittleEndian(void);

uint64 nextCommonCount(void);
char *makeTempPath(char *ext);
char *makeTempFile(char *ext);
char *makeTempDir(char *ext);
char *makeFreeDir(void);

char *getSelfFile(void);
char *getSelfDir(void);
uint getSelfProcessId(void);

char *getOutFile(char *localFile);
char *c_getOutFile(char *localFile);
char *c_getOutFile_x(char *localFile);
char *getOutFile_x(char *localFile);
void openOutDir(void);

autoList_t *tokenizeArgs(char *str);

int hasArgs(uint count);
int argIs(char *spell);
char *getArg(uint index);
char *nextArg(void);
char *nnNextArg(void);
void skipArg(uint count);
autoList_t *getFollowArgs(uint index);
autoList_t *allArgs(void);
uint getFollowArgCount(uint index);
uint getArgIndex(void);
void setArgIndex(uint index);

char *innerResPathFltr(char *path);

// ----

char *LOGPOS_Time(int mode);

// ----

void mkAppDataDir(void);
char *getAppDataEnv(char *name, char *defval);
uint getAppDataEnv32(char *name, uint defval);

int isWindows10(void);
int isWindows10orLater(void);
void getWindowsVer(uint ver[3]);
