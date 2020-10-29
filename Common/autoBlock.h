typedef struct autoBlock_st // static member
{
	uchar *Block;
	uint Size;
	uint AllocSize;
	uint BaseSize;
}
autoBlock_t;

autoBlock_t *createAutoBlock(void *block, uint size, uint allocSize);
autoBlock_t *copyAutoBlock(autoBlock_t *i);
void releaseAutoBlock(autoBlock_t *i);
void releaseBlock(autoBlock_t *i);

autoBlock_t *bindBlock(void *block, uint size);
autoBlock_t *recreateBlock(void *block, uint size);
autoBlock_t *createBlock(uint allocSize);
autoBlock_t *newBlock(void);
void *unbindBlock(autoBlock_t *i);

void setByte(autoBlock_t *i, uint index, uint byte);
uint getByte(autoBlock_t *i, uint index);
void putByte(autoBlock_t *i, uint index, uint byte);
uint refByte(autoBlock_t *i, uint index);
void swapByte(autoBlock_t *i, uint index1, uint index2);

void addByte(autoBlock_t *i, uint byte);
uint unaddByte(autoBlock_t *i);

void insertBytes(autoBlock_t *i, uint index, autoBlock_t *bytes);
void insertByteRepeat(autoBlock_t *i, uint index, uint byte, uint count);
void insertByte(autoBlock_t *i, uint index, uint byte);
autoBlock_t *desertBytes(autoBlock_t *i, uint index, uint count);
uint desertByte(autoBlock_t *i, uint index);
void removeBytes(autoBlock_t *i, uint index, uint count);

void fixBytes(autoBlock_t *i);
void setAllocSize(autoBlock_t *i, uint size);
void setSize(autoBlock_t *i, uint size);
uint getSize(autoBlock_t *i);

autoBlock_t *nobCreateBlock(uint size);
void nobSetSize(autoBlock_t *i, uint size);

autoBlock_t *bCreateBlock(uint size);
void clearBlock(autoBlock_t *i);
void resetSize(autoBlock_t *i, uint size);

autoBlock_t gndBlock(void *block, uint size);
autoBlock_t gndSubBytes(autoBlock_t *i, uint start, uint size);
autoBlock_t gndFollowBytes(autoBlock_t *i, uint start);
autoBlock_t *gndBlockVarPtr(void *block, uint size, autoBlock_t *varPtr);
autoBlock_t *gndSubBytesVarPtr(autoBlock_t *i, uint start, uint size, autoBlock_t *varPtr);
autoBlock_t *gndFollowBytesVarPtr(autoBlock_t *i, uint start, autoBlock_t *varPtr);

#define gndBlockVar(block, size, varname) \
	gndBlockVarPtr(block, size, &(varname))
#define gndSubBytesVar(i, start, size, varname) \
	gndSubBytesVarPtr(i, start, size, &(varname))
#define gndFollowBytesVar(i, start, varname) \
	gndFollowBytesVarPtr(i, start, &(varname))

#define gndBlockLine(line) \
	gndBlock(line, strlen(line))
#define gndBlockLineVarPtr(line, varPtr) \
	gndBlockVarPtr(line, strlen(line), varPtr)
#define gndBlockLineVar(line, varname) \
	gndBlockLineVarPtr(line, &(varname))

void *directGetBuffer(autoBlock_t *i);
uchar *directGetBlock(autoBlock_t *i, uint index, uint size);
void reverseBytes(autoBlock_t *i);
void addBytes(autoBlock_t *i, autoBlock_t *bytes);
autoBlock_t *unaddBytesRev(autoBlock_t *i, uint count);
autoBlock_t *unaddBytes(autoBlock_t *i, uint count);
void unaddBytesRevToBlock(autoBlock_t *i, void *block, uint size);
void unaddBytesToBlock(autoBlock_t *i, void *block, uint size);
autoBlock_t *getSubBytes(autoBlock_t *i, uint index, uint count);
autoBlock_t *getFollowBytes(autoBlock_t *i, uint index);
char *unbindBlock2Line(autoBlock_t *i);

/*
	chr = getByte(block, index);				chr = b_(block)[index];
	setByte(block, index, chr);					b_(block)[index] = chr;
	chr = getByte(block, 0);					chr = *b_(block);
	setByte(block, 0, chr);						*b_(block) = chr;
	setByte(block, a, getByte(block, b));		b_(block)[a] = b_(block)[b];
*/
#define b_(block) \
	((uchar *)(block)->Block)

uint findByteMatch(autoBlock_t *i, int (*match)(uint));
uint findByte(autoBlock_t *i, uint byte);
