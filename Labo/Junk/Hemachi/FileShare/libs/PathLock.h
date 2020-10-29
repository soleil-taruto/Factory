int HFS_NM_LockPath(char *path, int checkOnlyMode);
void HFS_NM_UnlockPath(char *path);

int HFS_LockPath(char *path, int checkOnlyMode);
void HFS_UnlockPath(char *path);
void HFS_UnlockAllPath(void);
