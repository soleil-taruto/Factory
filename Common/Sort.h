void gnomeSort(autoList_t *list, sint (*funcComp)(uint, uint));
void combSort(autoList_t *list, sint (*funcComp)(uint, uint));
void selectionSort(autoList_t *list, sint (*funcComp)(uint, uint));
void rapidSort(autoList_t *list, sint (*funcComp)(uint, uint));
void rapidSortLines(autoList_t *lines);
void rapidSortJLinesICase(autoList_t *lines);

void rapidSortSubColumns(autoList_t *list, sint (*funcComp)(uint, uint), autoList_t *subColumn, autoList_t *subColumns);
void rapidSortLinesSubColumns(autoList_t *lines, autoList_t *subColumn, autoList_t *subColumns);
void rapidSortLinesICaseSubColumns(autoList_t *lines, autoList_t *subColumn, autoList_t *subColumns);

sint strcmp3(char *line1, char *line2);
sint strcmp2(char *line1, char *line2);
sint simpleComp(uint v1, uint v2);
sint simpleComp64(uint64 v1, uint64 v2);
sint pSimpleComp(uint v1, uint v2);
sint pSimpleComp2(uint v1, uint v2);

#define m_simpleComp(v1, v2) \
	((v1) < (v2) ? -1 : (v2) < (v1) ? 1 : 0)

uint binSearch(autoList_t *list, uint target, sint (*funcComp)(uint, uint));
uint binSearchLines(autoList_t *lines, char *lineFind);
uint findBoundNearestMatch(autoList_t *list, uint target, sint (*funcComp)(uint, uint));
uint findBoundNearestRight(autoList_t *list, uint target, sint (*funcComp)(uint, uint));
void findBound(autoList_t *list, uint target, sint (*funcComp)(uint, uint), uint bound[2]);
int getBound(autoList_t *list, uint target, sint (*funcComp)(uint, uint), uint bound[2]);
int getBoundLines(autoList_t *lines, char *lineFind, uint bound[2]);
