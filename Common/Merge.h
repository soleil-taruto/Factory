autoList_t *merge(autoList_t *list1, autoList_t *list2, sint (*funcComp)(uint, uint), void (*funcRelease)(uint));
autoList_t *mergeLines(autoList_t *lines1, autoList_t *lines2);
autoList_t *mergeConstLines(autoList_t *lines1, autoList_t *lines2);
autoList_t *mergeLinesICase(autoList_t *lines1, autoList_t *lines2);
autoList_t *mergeConstLinesICase(autoList_t *lines1, autoList_t *lines2);

autoList_t *selectLines(autoList_t *lines);
char *selectLine(autoList_t *lines);

extern autoList_t *merge2_bothExist2;

void merge2(autoList_t *list1, autoList_t *list2, sint (*funcComp)(uint, uint), autoList_t *onlyExist1, autoList_t *bothExist, autoList_t *onlyExist2);
void mergeLines2(autoList_t *list1, autoList_t *list2, autoList_t *onlyExist1, autoList_t *bothExist, autoList_t *onlyExist2);
void mergeLines2ICase(autoList_t *lines1, autoList_t *lines2, autoList_t *onlyExist1, autoList_t *bothExist, autoList_t *onlyExist2);

void distinct(autoList_t *list, sint (*funcComp)(uint, uint), autoList_t *result, autoList_t *others);
void distinctLines(autoList_t *lines, autoList_t *result, autoList_t *others);
void distinctJLinesICase(autoList_t *lines, autoList_t *result, autoList_t *others);

autoList_t *autoDistinctLines(autoList_t *lines);
autoList_t *autoDistinctJLinesICase(autoList_t *lines);

void distinct2(autoList_t *list, sint (*funcComp)(uint, uint), void (*funcRelease)(uint));

autoList_t *selectLines_x(autoList_t *lines);
char *selectLine_x(autoList_t *lines);
