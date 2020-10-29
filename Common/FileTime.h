void getFileStamp(char *file, uint64 *createTime, uint64 *accessTime, uint64 *updateTime);
void setFileStamp(char *file, uint64 createTime, uint64 accessTime, uint64 updateTime);
uint64 getFileStampByTime(time_t t);
time_t getTimeByFileStamp(uint64 stamp);
