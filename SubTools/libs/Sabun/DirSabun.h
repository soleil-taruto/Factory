typedef enum DirSabunKind_et
{
	DSK_LONLY_DIR = 2,
	DSK_LONLY_FILE,
	DSK_RONLY_DIR,
	DSK_RONLY_FILE,
	DSK_BOTH_DIR,
	DSK_BOTH_FILE,
}
DirSabunKind_t;

typedef struct DirSabun_st
{
	char *Path;
	DirSabunKind_t Kind;
	autoList_t *LDiff;
	autoList_t *RDiff;
}
DirSabun_t;

DirSabun_t *CreateDirSabun(char *path, DirSabunKind_t kind, autoList_t *lDiff, autoList_t *rDiff);
void ReleaseDirSabun(DirSabun_t *i);
void ReleaseDirSabunList(autoList_t *list);

autoList_t *MakeDirSabun(char *lDir, char *rDir, int lWithData, int rWithData);
