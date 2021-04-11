#include "all.h"

void stdin_set_bin(void);
void stdin_set_text(void);

void termination_fileCloseAll(void);

FILE *rfopen(char *file, char *mode);
FILE *fileOpen(char *file, char *mode);
void fileClose(FILE *fp);

HANDLE getHandleByFilePointer(FILE *fp);

void fileSeek(FILE *fp, int origin, sint64 offset);
void fileRead(FILE *fp, autoBlock_t *block);
void fileWrite(FILE *fp, autoBlock_t *block);

autoBlock_t *readBinaryStream(FILE *fp, uint size);
autoBlock_t *readBinaryBlock(FILE *fp, uint size);
autoBlock_t *neReadBinaryBlock(FILE *fp, uint size);
autoBlock_t *readBinary(char *file);
autoBlock_t *readBinaryToEnd(FILE *fp, autoBlock_t *buff);
void readWriteBinary(FILE *rfp, FILE *wfp, uint64 size);
void readWriteBinaryToEnd(FILE *rfp, FILE *wfp);
void writeBinaryBlock(FILE *fp, autoBlock_t *block);
void writeBinary(char *file, autoBlock_t *block);
void writeJoinBinary(char *file, autoBlock_t *block);

int readChar(FILE *fp);
int neReadChar(FILE *fp);
char *readLine(FILE *fp);
char *readLineLenMax(FILE *fp, uint lenmax);
char *nnReadLine(FILE *fp);
char *neReadLine(FILE *fp);
char *readLine_strr(FILE *fp);
autoList_t *readLines(char *file);
char *readText(char *file);
char *readText_b(char *file);
void writeChar(FILE *fp, int chr);
void writeToken(FILE *fp, char *line);
void writeLine(FILE *fp, char *line);
void writeLines2Stream(FILE *fp, autoList_t *lines);
void writeLines(char *file, autoList_t *lines);
void writeLines_b(char *file, autoList_t *lines);
void addLines2File(char *file, autoList_t *lines);

char *readFirstLine(char *file);
void writeOneLine(char *file, char *line);
void writeOneLineNoRet(char *file, char *line);
void writeOneLineNoRet_b(char *file, char *line);
void addLine2File(char *file, char *line);
void addLine2FileNoRet(char *file, char *line);
void addLine2FileNoRet_b(char *file, char *line);

void writeValue64Width(FILE *fp, uint64 value, uint width);
void writeValue64(FILE *fp, uint64 value);
void writeValueWidth(FILE *fp, uint value, uint width);
void writeValue(FILE *fp, uint value);
uint64 readValue64Width(FILE *fp, uint width);
uint64 readValue64(FILE *fp);
uint readValueWidth(FILE *fp, uint width);
uint readValue(FILE *fp);

uint readFirstValue(char *file);
void writeOneValue(char *file, uint value);
uint64 readFirstValue64(char *file);
void writeOneValue64(char *file, uint64 value);

typedef struct BUFF_st // static member
{
	FILE *Fp;
	autoBlock_t *Buffer;
	uint BuffSize;
	uint Index;
}
BUFF;
BUFF *buffBind(FILE *fp, uint buffSize);
FILE *buffUnbind(BUFF *bp);
BUFF *buffOpen(char *file, char *mode, uint buffSize);
void buffClose(BUFF *bp);
int buffReadChar(BUFF *bp);
void buffUnreadChar(BUFF *bp, int chr);
void buffUnreadBuffer(BUFF *bp);

// c_
char *c_nnReadLine(FILE *fp);
char *c_neReadLine(FILE *fp);

// _x
FILE *fileOpen_cx(char *file, char *mode);
FILE *fileOpen_xc(char *file, char *mode);
FILE *fileOpen_xx(char *file, char *mode);
autoList_t *readLines_x(char *file);
char *readText_x(char *file);
void writeToken_x(FILE *fp, char *line);
void writeLine_x(FILE *fp, char *line);
void writeOneLine_cx(char *file, char *line);
void writeOneLine_xc(char *file, char *line);
void writeOneLine_xx(char *file, char *line);
void addLine2File_cx(char *file, char *line);
void writeLines2Stream_x(FILE *fp, autoList_t *lines);
void writeLines_cx(char *file, autoList_t *lines);
void writeLines_xc(char *file, autoList_t *lines);
void writeLines_xx(char *file, autoList_t *lines);
void writeLines_b_cx(char *file, autoList_t *lines);
void addLines2File_cx(char *file, autoList_t *lines);
void writeBinaryBlock_x(FILE *fp, autoBlock_t *block);
void writeBinary_cx(char *file, autoBlock_t *block);
void writeBinary_xx(char *file, autoBlock_t *block);
void writeJoinBinary_cx(char *file, autoBlock_t *block);
void writeOneLineNoRet_cx(char *file, char *line);
void writeOneLineNoRet_b_cx(char *file, char *line);
void writeOneLineNoRet_b_xc(char *file, char *line);
void writeOneLineNoRet_b_xx(char *file, char *line);
