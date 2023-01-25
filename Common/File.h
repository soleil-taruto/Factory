void trimPath(char *path);
char *combine(char *dir, char *file);

char *putYen(char *path);
char *unputYen(char *path);

char *getLocalWithYen(char *path);
char *getLocal(char *path);
char *getExtWithDot(char *path);
char *getExt(char *path);
char *changeLocal(char *path, char *newLocal);
char *getParent(char *path);
char *changeExt(char *path, char *newExt);
void eraseLocal(char *path);
void eraseExt(char *path);
char *addLocal(char *path, char *newLocal);
char *addExt(char *path, char *newExt);

#define isAbsRootDir(absPath) \
	(strlen(absPath) == 3)

int isAbsPath(char *path);
int isRootDirAbs(char *path);

char *makeFullPath(char *path);
int isRootDir(char *path);
char *getFullPath(char *path, char *baseDir);

int accessible(char *path);
int existPath(char *path);
int existDir(char *dir);
int existFile(char *file);

sint dirFileComp(char *path1, char *path2);
void dirFileSort(autoList_t *paths);
void dirFileSortDirCount(autoList_t *paths, uint dirCount);

int mkdirEx(char *dir);

int creatable(char *path);
int overwritable(char *path);
void createDir(char *dir);
void createFile(char *file);
void createDirIfNotExist(char *dir);
void createFileIfNotExist(char *file);
void removeDir(char *dir);
void removeFile(char *file);
void removeDirIfExist(char *dir);
void removeDirIfExistEmpty(char *dir);
void removeDirIfEmpty(char *dir);
void removeFileIfExist(char *file);
void removePath(char *path);
void removePathIfExist(char *path);
void recurClearDir(char *dir);
void recurRemoveDir(char *dir);
void recurRemoveDirIfExist(char *dir);
void recurRemovePath(char *path);
void recurRemovePathIfExist(char *path);

void createPath(char *path, int mode);

char *getCwd(void);
void changeCwd(char *dir);
void addCwd(char *dir);
void unaddCwd(void);
void mkAddCwd(char *dir);
void unaddAllCwd(void);

char *eraseRoot(char *path, char *root);
void eraseRoots(autoList_t *pathList, char *root);
char *changeRoot(char *path, char *root, char *newRoot);
void changeRoots(autoList_t *pathList, char *root, char *newRoot);
void changeRootsAbs(autoList_t *pathList, char *root, char *newRoot);
int isChangeableRoot(char *path, char *root);
void eraseParent(char *path);
void eraseParents(autoList_t *pathList);

extern int (*userIsCancel_CopyFile_DM)(char *srcFile, char *destFile, char *destMode);

void joinFile(char *bodyFile, char *tailFile);
void copyFile(char *srcFile, char *destFile);
void copyDir(char *srcDir, char *destDir);
void moveFile(char *srcFile, char *destFile);
void moveDir(char *srcDir, char *destDir);

void copyPath(char *rPath, char *wPath);
void movePath(char *rPath, char *wPath);

void setFileSizeFP(FILE *fp, uint64 size);
void setFileSize(char *file, uint64 size);
uint64 getFileSizeFPSS(FILE *fp);
uint64 getFileSizeFP(FILE *fp);
uint64 getFileSize(char *file);
uint64 getSeekPos(FILE *fp);
uint64 getDirSize(char *dir);

void editTextFile(char *file);
autoList_t *editTextLines(autoList_t *lines);
void viewTextLines(autoList_t *lines);
void semiRemovePath(char *path);

void removeFileAtTermination(char *file);
void termination_scheduledRemoveFile(void);

// c_
char *c_getCwd(void);
char *c_changeLocal(char *path, char *newLocal);
char *c_getParent(char *path);
char *c_changeExt(char *path, char *newExt);
char *c_combine(char *dir, char *file);
char *c_makeFullPath(char *path);

// _x
char *changeLocal_xc(char *path, char *newLocal);
char *changeLocal_cx(char *path, char *newLocal);
void changeRoots_xc(autoList_t *pathList, char *root, char *newRoot);
char *changeExt_xc(char *path, char *newExt);
char *changeExt_cx(char *path, char *newExt);
char *combine_cx(char *dir, char *file);
char *combine_xc(char *dir, char *file);
char *combine_xx(char *dir, char *file);
char *makeFullPath_x(char *path);
void removeDir_x(char *dir);
void removeFile_x(char *file);
void removeDirIfEmpty_x(char *dir);
void removeFileIfExist_x(char *file);
void removePath_x(char *path);
void removePathIfExist_x(char *path);
void recurClearDir_x(char *dir);
void recurRemoveDirIfExist_x(char *dir);
void recurRemovePath_x(char *path);
void recurRemovePathIfExist_x(char *path);
void recurRemoveDir_x(char *dir);
void copyFile_cx(char *srcFile, char *destFile);
void copyFile_xc(char *srcFile, char *destFile);
void copyFile_xx(char *srcFile, char *destFile);
void moveDir_cx(char *srcDir, char *destDir);
void moveDir_xc(char *srcDir, char *destDir);
void moveDir_xx(char *srcDir, char *destDir);
char *getParent_x(char *path);
autoList_t *editTextLines_x(autoList_t *lines);
void addCwd_x(char *dir);
void mkAddCwd_x(char *dir);
void createFile_x(char *file);
void createDirIfNotExist_x(char *dir);
void createFileIfNotExist_x(char *file);
void createPath_x(char *path, int mode);
