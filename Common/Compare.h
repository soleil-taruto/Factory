int isSameBlock(autoBlock_t *block1, autoBlock_t *block2);
int isSameList(autoList_t *list1, autoList_t *list2);
int isSameFile(char *file1, char *file2);
int isSameDir(char *dir1, char *dir2, int ignoreCase);
int isSameLines(autoList_t *lines1, autoList_t *lines2, int ignoreCase);
int isSameLinesList(autoList_t *table1, autoList_t *table2, int ignoreCase);

autoList_t *getDiffLinesReport(autoList_t *lines1, autoList_t *lines2, int ignoreCase);
autoList_t *getDiffLinesReportLim(autoList_t *lines1, autoList_t *lines2, int ignoreCase, uint lineNumLimit);
