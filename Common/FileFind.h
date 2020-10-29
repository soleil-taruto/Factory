typedef struct lsInfo_st
{
	uint attrArch : 1;
	uint attrHidden : 1;
	uint attrReadOnly : 1;
	uint attrSubDir : 1;
	uint attrSystem : 1;
	time_t createTime;
	time_t accessTime;
	time_t writeTime;
	// size ÇÕ _fsize_t Ç™ 32 bit Ç»ÇÃÇ≈ñ≥éãÇ∑ÇÈÅB
}
lsInfo_t;

extern void (*lsDirAction)(char *dir);
extern void (*lsFileAction)(char *file);
extern autoList_t *lsInfos;
extern int antiSubversion;
extern int ignoreUtfPath;
extern uint findLimiter;
extern int (*findAcceptName)(char *name);
extern struct _finddata_t lastFindData;
extern uint lastDirCount;

autoList_t *ls(char *dir);
autoList_t *lss(char *dir);

void updateFindData(char *path);
int tryUpdateFindData(char *path);

time_t getFileAccessTime(char *file);
time_t getFileCreateTime(char *file);
time_t getFileModifyTime(char *file);
time_t getFileWriteTime(char *file);

autoList_t *lsFiles(char *dir);
autoList_t *lsDirs(char *dir);
autoList_t *lssFiles(char *dir);
autoList_t *lssDirs(char *dir);

void ls2File(char *dir, char *dirsFile, char *filesFile);
void lss2File(char *dir, char *dirsFile, char *filesFile);

void fileSearch(char *wCard, int (*action)(struct _finddata_t *));
uint fileSearchCount(char *wCard);
int fileSearchExist(char *wCard);

uint lsCount(char *dir);

// ---- cmdDir ----

void cmdDir_ls2File_noClear(char *dir, char *dirsFile, char *filesFile);
void cmdDir_ls2File(char *dir, char *dirsFile, char *filesFile);
void cmdDir_lss2File(char *dir, char *dirsFile, char *filesFile);
autoList_t *cmdDir_lsFiles(char *dir);
autoList_t *cmdDir_lsDirs(char *dir);
autoList_t *cmdDir_lssFiles(char *dir);
autoList_t *cmdDir_lssDirs(char *dir);

// ----
