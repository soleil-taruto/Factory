void ab_addBlock(autoBlock_t *i, void *block, uint size);
void ab_addBlock_x(autoBlock_t *i, void *block, uint size);
void ab_addBytes(autoBlock_t *i, autoBlock_t *block);
void ab_addBytes_x(autoBlock_t *i, autoBlock_t *block);
void ab_addSubBytes(autoBlock_t *i, autoBlock_t *block, uint start, uint size);
void ab_addSubBytes_x(autoBlock_t *i, autoBlock_t *block, uint start, uint size);
void ab_addFollowBytes(autoBlock_t *i, autoBlock_t *block, uint start);
void ab_addFollowBytes_x(autoBlock_t *i, autoBlock_t *block, uint start);
void ab_addLine(autoBlock_t *i, char *line);
void ab_addLine_x(autoBlock_t *i, char *line);

#define ab_addChar(i, chr) \
	(addByte((i), (chr)))

void ab_addValue(autoBlock_t *i, uint value);
uint ab_unaddValue(autoBlock_t *i);
void ab_insertValue(autoBlock_t *i, uint index, uint value);
uint ab_desertValue(autoBlock_t *i, uint index);
void ab_setValue(autoBlock_t *i, uint index, uint value);
uint ab_getValue(autoBlock_t *i, uint index);
void ab_putValue(autoBlock_t *i, uint index, uint value);
uint ab_refValue(autoBlock_t *i, uint index);

uint ab_getValueBE(autoBlock_t *i, uint index);

void *ab_makeBlock(autoBlock_t *i);
void *ab_makeBlock_x(autoBlock_t *i);
autoBlock_t *ab_makeSubBytes(autoBlock_t *i, uint start, uint size);
autoBlock_t *ab_makeSubBytes_x(autoBlock_t *i, uint start, uint size);
autoBlock_t *ab_makeFollowBytes(autoBlock_t *i, uint start);
autoBlock_t *ab_makeFollowBytes_x(autoBlock_t *i, uint start);
char *ab_makeLine(autoBlock_t *i);
char *ab_makeLine_x(autoBlock_t *i);
autoBlock_t *ab_makeBlockLine(char *line);
autoBlock_t *ab_makeBlockLine_x(char *line);

// ---- バイト列 from_to 文字列 ----

#define ab_fromLine(line) \
	(ab_makeBlockLine((line)))

#define ab_fromLine_x(line) \
	(ab_makeBlockLine_x((line)))

#define ab_toLine(block) \
	(ab_makeLine((block)))

#define ab_toLine_x(block) \
	(ab_makeLine_x((block)))

// ---- バイト列 from_to 16進_文字列 ----

#define ab_fromHexLine(line) \
	(makeBlockHexLine((line)))

#define ab_fromHexLine_x(line) \
	(makeBlockHexLine_x((line)))

#define ab_toHexLine(block) \
	(makeHexLine((block)))

#define ab_toHexLine_x(block) \
	(makeHexLine_x((block)))

// ----

void ab_addTrailZero(autoBlock_t *i, uint count);
uint ab_unaddTrailZero(autoBlock_t *i);

void ab_swap(autoBlock_t *i, autoBlock_t *j);
autoBlock_t *ab_eject(autoBlock_t *i);
void ab_uneject(autoBlock_t *i, autoBlock_t *j);
