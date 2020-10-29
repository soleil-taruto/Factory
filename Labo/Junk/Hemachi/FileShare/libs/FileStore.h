extern char *HFS_StoreDir;
extern uint64 HFS_DiskFreeSpaceMin;
extern uint64 HFS_StoreDirSizeMax;

void HFS_KeepDiskFreeSpace(void);
int HFS_Perform(char *prmFile, char *ansFile);

char *HFS_GetRealPath(char *virPath);
void HFS_ReleaserealPath(char *realPath);
