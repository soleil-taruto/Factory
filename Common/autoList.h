typedef struct autoList_st // static member
{
	uint *Elements;
	uint Count;
	uint AllocCount : 30; // Count, AllocCount ‚Ì(ŽÀ‘•ã‚Ì)Å‘å’l‚Å‚ ‚é UINTMAX / sizeof(uint) ‚ªƒMƒŠƒMƒŠ“ü‚éB
	uint Unresizable : 1;
	uint Reserved_01 : 1;
}
autoList_t;

autoList_t *createAutoList(uint allocCount);
autoList_t *copyAutoList(autoList_t *i);
void releaseAutoList(autoList_t *i);
void releaseList(autoList_t *i);

autoList_t *bindAutoList(uint *list, uint count);
uint *unbindAutoList(autoList_t *i);
autoList_t *recreateAutoList(uint *list, uint count);
autoList_t *createOneElement(uint element);
autoList_t *newList(void);

autoList_t *nobCreateList(uint count);
void nobSetCount(autoList_t *i, uint count);

autoList_t *bnewList(uint count);
void clearList(autoList_t *i);
void resetCount(autoList_t *i, uint count);

autoList_t gndAutoList(uint *list, uint count);
autoList_t gndSubElements(autoList_t *i, uint start, uint count);
autoList_t gndFollowElements(autoList_t *i, uint start);
autoList_t gndOneElement(uint element, uint *elementBox);
autoList_t *gndAutoListVarPtr(uint *list, uint count, autoList_t *varPtr);
autoList_t *gndSubElementsVarPtr(autoList_t *i, uint start, uint count, autoList_t *varPtr);
autoList_t *gndFollowElementsVarPtr(autoList_t *i, uint start, autoList_t *varPtr);
autoList_t *gndOneElementVarPtr(uint element, uint *elementBox, autoList_t *varPtr);

#define gndAutoListVar(list, count, varname) \
	gndAutoListVarPtr(list, count, &(varname))
#define gndSubElementsVar(i, start, count, varname) \
	gndSubElementsVarPtr(i, start, count, &(varname))
#define gndFollowElementsVar(i, start, varname) \
	gndFollowElementsVarPtr(i, start, &(varname))
#define gndOneElementVar(element, buffvarname, varname) \
	gndOneElementVarPtr(element, &(buffvarname), &(varname))

void setElement(autoList_t *i, uint index, uint element);
uint getElement(autoList_t *i, uint index);
void putElement(autoList_t *i, uint index, uint element);
uint refElement(autoList_t *i, uint index);
uint getLastElement(autoList_t *i);
void swapElement(autoList_t *i, uint index1, uint index2);

void addElement(autoList_t *i, uint element);
uint unaddElement(autoList_t *i);

void insertElement(autoList_t *i, uint index, uint element);
uint desertElement(autoList_t *i, uint index);
uint fastDesertElement(autoList_t *i, uint index);
void removeElement(autoList_t *i, uint target);

void fixElements(autoList_t *i);
void setAllocCount(autoList_t *i, uint count);
void setCount(autoList_t *i, uint count);
void setCountFloor(autoList_t *i, uint count);
void setCountRoof(autoList_t *i, uint count);
uint getCount(autoList_t *i);

uint *directGetList(autoList_t *i);
uint *directGetPoint(autoList_t *i, uint index);
uint *directRefPoint(autoList_t *i, uint index);
void reverseElements(autoList_t *i);
void removeZero(autoList_t *i);
void removeTrailZero(autoList_t *i);
void releaseDim_BR(void *block, uint depth, void (*blockReleaser)(void *));
void releaseDim(void *block, uint depth);
void addElements(autoList_t *i, autoList_t *j);
uint findElement(autoList_t *i, uint target, sint (*funcComp)(uint, uint));
uint getCountElement(autoList_t *i, uint target, sint (*funcComp)(uint, uint));
extern uint foundPairIndexes[2];
uint findPair(autoList_t *list, sint (*funcComp)(uint, uint));

autoList_t *getList(autoList_t *table, uint index);
autoList_t *refList(autoList_t *table, uint index);
autoList_t *makeTable(uint rowcnt, uint colcnt, uint initValue);
uint *getTablePoint(autoList_t *table, uint rowidx, uint colidx);

void callAllElement(autoList_t *i, void (*callFunc)(uint));
uint zSetElement(autoList_t *i, uint index, uint element);

// _x
void addElements_x(autoList_t *i, autoList_t *j);
void callAllElement_x(autoList_t *i, void (*callFunc)(uint));

/*
	value = getElement(list, index);				value = e_(list)[index];
	setElement(list, index, value);					e_(list)[index] = value;
	value = getElement(list, 0);					value = *e_(list);
	setElement(list, 0, value);						*e_(list) = value;
	setElemenet(list, a, getElement(list, b));		e_(list)[a] = e_(list)[b];
*/
#define e_(list) \
	((uint *)(list)->Elements)
