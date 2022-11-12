#include "all.h"

void stdin_set_bin(void)
{
	errorCase(_setmode(_fileno(stdin), _O_BINARY) == -1); // ? ���s
}
void stdin_set_text(void) // �e�L�X�g���[�h���f�t�H���g�Ȃ̂Ō��ɖ߂��B
{
	errorCase(_setmode(_fileno(stdin), _O_TEXT) == -1); // ? ���s
}

static autoList_t *OpenedFPList;

static void AddOpenedFP(FILE *fp)
{
	if (!OpenedFPList)
		OpenedFPList = newList();

	addElement(OpenedFPList, (uint)fp);
}
static void RemoveOpenedFP(FILE *fp)
{
	uint rmPos;

	errorCase(!OpenedFPList);
	rmPos = findElement(OpenedFPList, (uint)fp, simpleComp);
	errorCase(rmPos == getCount(OpenedFPList));
	fastDesertElement(OpenedFPList, rmPos);
}
void termination_fileCloseAll(void)
{
	if (OpenedFPList)
	{
		FILE *fp;
		uint index;

		foreach (OpenedFPList, fp, index)
		{
			LOGPOS();
			fclose(fp);
		}
		releaseAutoList(OpenedFPList);
		OpenedFPList = NULL;
	}
}

FILE *rfopen(char *file, char *mode)
{
	uint retrycnt;

	for (retrycnt = 0; ; retrycnt++)
	{
		// fopen() �͈����ɖ�肪�����Ă��A�H�Ɏ��s���邱�Ƃ�����B
		// �K���ɊԊu���󂯂ĉ��x�����g���C����B
		FILE *fp = fopen(file, mode); // file == "" �̂Ƃ� NULL ��Ԃ��B

		if (fp != NULL)
		{
			if (retrycnt)
			{
				cout("File opened at %u-th retrial.\n", retrycnt);
			}
			return fp;
		}
		if (retrycnt % 5 == 0)
		{
			cout("Can not open file \"%s\" as \"%s\" mode, %u-th trial.\n", file, mode, retrycnt + 1);

			if (20 <= retrycnt)
			{
				return NULL;
			}
		}
		sleep(100);
	}
}
FILE *fileOpen(char *file, char *mode)
{
	FILE *fp = rfopen(file, mode);

	errorCase(!fp);
	AddOpenedFP(fp);
	return fp;
}
void fileClose(FILE *fp)
{
	RemoveOpenedFP(fp);

	if (fclose(fp) != 0)
	{
		error();
	}
}

HANDLE getHandleByFilePointer(FILE *fp) // ret: CreateFile() �ɂ���ē����� HANDEL �Ɠ���
{
	return (HANDLE)_get_osfhandle(fileno(fp));
}

/*
	fileSeek() ����ꍇ�� fileOpen() �̃��[�h
		�ǂݍ��� "rb"
		�ǂݏ��� "r+b"
		�ǋL     "r+b" �ŊJ���āAfileSeek(fp, SEEK_END, 0);

		w+ �I�[�v�����Ƀt�@�C������ɂ���B
		a+ �I�[�v�����ɏI�[�� 0x1a ���폜����B("a+b" �ł͍폜���Ȃ�)
		t  �V�[�N�ɐ���

	origin
		SEEK_SET �ŏ��̃o�C�g�ʒu
		SEEK_CUR ���݈ʒu
		SEEK_END �Ō�̃o�C�g�̎��̃o�C�g�ʒu

	SEEK_END ����̗̈�ɂ��V�[�N�ł���悤���B
	�V�[�N���������ł̓t�@�C���͊g�����ꂸ�A�������ނƊg�������΂��������� 0x00 �ŕ�U�����B
	�ǂݍ��ނ� EOF ��Ԃ��B

	�t�@�C���̐擪���O�ɃV�[�N���悤�Ƃ���ƃG���[�ɂȂ�B(_fseeki64 �� 0 �ȊO��Ԃ�)

	r+b�ŃI�[�v�����̒��� -- ����͎d�l�炵���B

		�ǂݍ���ł��珑���o���O�ɃV�[�N���Ȃ���΂Ȃ�Ȃ��B

			...
			readChar(fp);
			writeChar(fp, c);            // <- �������܂�Ȃ��I
			...

			...
			readChar(fp);
			fileSeek(fp, SEEK_CUR, 0);
			writeChar(fp, c);            // <- �������܂��B
			...

		�����o���Ă���ǂݍ��ޑO�ɂ��V�[�N���Ȃ���΂Ȃ�Ȃ����ۂ��B

			writeChar(fp, c);
			fileSeek(fp, SEEK_CUR, 0);
			readChar(fp);

*/
void fileSeek(FILE *fp, int origin, sint64 offset) // origin, offset �̕��т� fseek() �Ƌt
{
	if (_fseeki64(fp, offset, origin) != 0)
	{
		error();
	}
}
void fileRead(FILE *fp, autoBlock_t *block) // getSize(block) �o�C�g�ǂݍ��ޑO�� EOF �ɒB�����ꍇ���G���[
{
	uint size = getSize(block);

	if (size)
	{
		uint retsize = fread(directGetBuffer(block), 1, size, fp);

		errorCase(retsize != size);
		errorCase(ferror(fp));
	}
}
void fileWrite(FILE *fp, autoBlock_t *block)
{
	uint size = getSize(block);

	if (size)
	{
		uint retsize = fwrite(directGetBuffer(block), 1, size, fp);

		errorCase(retsize != size);
		errorCase(ferror(fp));
	}
}

/*
	����
		EOF �̒��O�܂��� EOF �ɒB���Ă��� 0 �o�C�g�ǂݍ��� -> 0 �o�C�g�̃u���b�N��Ԃ��B
		EOF �̒��O�܂��� EOF �ɒB���Ă��� 1 �o�C�g�ǂݍ��� -> NULL ��Ԃ��B
*/
autoBlock_t *readBinaryStream(FILE *fp, uint size) // size == 0 �ł��B�ǂݏI���� NULL ��Ԃ��B
{
	void *block = memAlloc(size);
	uint readSize;

	// size == 0 �̂Ƃ� 0 ��Ԃ��B
	// EOF �̒��O�܂��� EOF �ɒB���Ă���ꍇ 0 ��Ԃ��B
	readSize = fread(block, 1, size, fp);

	if (size < readSize || (!readSize || readSize < size) && ferror(fp))
	{
		error();
	}
//	if (!readSize && size) // �������ł��ǂ���łȂ��H
	if (!readSize && feof(fp))
	{
		memFree(block);
		return NULL;
	}
	return bindBlock(block, readSize);
}
autoBlock_t *readBinaryBlock(FILE *fp, uint size) // NULL ��Ԃ��Ȃ��B
{
	autoBlock_t *block = readBinaryStream(fp, size);

	if (!block)
		block = createBlock(0);

	return block;
}
autoBlock_t *neReadBinaryBlock(FILE *fp, uint size) // NULL �̂Ƃ��� error();
{
	autoBlock_t *block = readBinaryStream(fp, size);

	errorCase(!block);
	return block;
}
autoBlock_t *readBinary(char *file)
{
	uint64 fileSize = getFileSize(file);
	FILE *fp;
	autoBlock_t *fileImage;

	errorCase(UINTMAX < fileSize); // ? �傫������B

	fp = fileOpen(file, "rb");
	fileImage = readBinaryBlock(fp, (uint)fileSize);
	fileClose(fp);

	return fileImage;
}
autoBlock_t *readBinaryToEnd(FILE *fp, autoBlock_t *buff) // buff: NULL == �V�����o�b�t�@�𐶐�, ret: buff
{
	errorCase(!fp); // 2bs

	if (!buff)
		buff = newBlock();

	for (; ; )
	{
		autoBlock_t *tmp = readBinaryStream(fp, 1024 * 1024 * 16);

		if (!tmp)
			break;

		ab_addBytes_x(buff, tmp);
	}
	return buff;
}

#define RWB_BUFF_SIZE (1024 * 1024 * 16)

void readWriteBinary(FILE *rfp, FILE *wfp, uint64 size)
{
	uchar *buff = memAlloc(RWB_BUFF_SIZE);
	uint64 count;

	for (count = 0; count < size; )
	{
		uint rwSize = m_min(RWB_BUFF_SIZE, size - count);
		uint readSize;
		uint wroteSize;

		readSize = fread(buff, 1, rwSize, rfp);

		errorCase(readSize != rwSize);
		errorCase(ferror(rfp));

		wroteSize = fwrite(buff, 1, rwSize, wfp);

		errorCase(wroteSize != rwSize);
		errorCase(ferror(wfp));

		count += rwSize;
	}
	memFree(buff);
}
void readWriteBinaryToEnd(FILE *rfp, FILE *wfp)
{
	uint64 rPos = getSeekPos(rfp);
	uint64 size;

	size = getFileSizeFP(rfp);
	fileSeek(rfp, SEEK_SET, rPos);
	size -= rPos;
	readWriteBinary(rfp, wfp, size);
}
void writeBinaryBlock(FILE *fp, autoBlock_t *block) // getSize(block) == 0 �ł���
{
	uint ret;

	if ((ret = fwrite(directGetBuffer(block), 1, getSize(block), fp)) != getSize(block)) // fwrite(, 1, 0, ) �̂Ƃ� 0 ��Ԃ��B
	{
		cout("fwrite error, ret: %u, block_size: %u, LastError: %08x\n", ret, getSize(block), GetLastError());
		error();
	}
}
static void WriteBinary_DM(char *file, autoBlock_t *block, char *destMode)
{
	FILE *fp = fileOpen(file, destMode);

	writeBinaryBlock(fp, block);
	fileClose(fp);
}
void writeBinary(char *file, autoBlock_t *block)
{
	WriteBinary_DM(file, block, "wb");
}
void writeJoinBinary(char *file, autoBlock_t *block)
{
	WriteBinary_DM(file, block, "ab");
}

int readChar(FILE *fp) // �o�C�i���E�e�L�X�g��킸�X�g���[������P�o�C�g(�P����)�ǂݍ��ށB
{
	int chr = fgetc(fp);

	if (chr == EOF && ferror(fp))
	{
		error();
	}
	return chr;
}
int neReadChar(FILE *fp)
{
	int chr = readChar(fp);

	errorCase(chr == EOF);
	return chr;
}
char *readLineLenMax(FILE *fp, uint lenmax)
{
	autoBlock_t *lineBuff = createBlock(128);
	char *line;
	int chr;

	for (; ; )
	{
		chr = readChar(fp);

		if (chr == '\r')
		{
			if (readChar(fp) != '\n') // ? CR-(not_LF)
			{
				cout("Warning: Reading antiquity MAC file! %d\n", fp);
			}
			break;
		}
		if (chr == '\n' || chr == EOF)
		{
			break;
		}
		if (lenmax <= getSize(lineBuff))
		{
			cout("Warning: Text line overflow! %d, Ignore 0x%02x and after.\n", fp, chr);
			break;
		}
		if (chr == '\0')
		{
			chr = '\1';
		}
		addByte(lineBuff, chr);
	}
	addByte(lineBuff, '\0');
	line = unbindBlock(lineBuff);

	if (line[0] == '\0' && chr == EOF)
	{
		memFree(line);
		line = NULL;
	}
	return line;
}
char *readLine(FILE *fp)
{
	return readLineLenMax(fp, UINTMAX);
}
char *readLine_strr(FILE *fp)
{
	char *line = readLine(fp);

	if (line)
		line = strr(line);

	return line;
}
char *nnReadLine(FILE *fp)
{
	char *line = readLine(fp);

	if (!line)
		line = strx("");

	return line;
}
char *neReadLine(FILE *fp)
{
	char *line = readLine(fp);
	errorCase(!line);
	return line;
}
autoList_t *readLines(char *file)
{
	FILE *fp = fileOpen(file, "rt");
	char *line;
	autoList_t *lines = createAutoList(16);

	while (line = readLine(fp))
	{
		addElement(lines, (uint)line);
	}
	fileClose(fp);
	return lines;
}
char *readText(char *file)
{
	FILE *fp = fileOpen(file, "rt");
	autoBlock_t *text = newBlock();

	for (; ; )
	{
		int chr = readChar(fp);

		if (chr == EOF)
		{
			break;
		}
		addByte(text, chr);
	}
	fileClose(fp);
	return unbindBlock2Line(text);
}
char *readText_b(char *file)
{
	return unbindBlock2Line(readBinary(file));
}
void writeChar(FILE *fp, int chr) // �o�C�i���E�e�L�X�g��킸�X�g���[���ɂP�o�C�g(�P����)�����o���B
{
	if (fputc(chr, fp) == EOF)
	{
		error();
	}
}
void writeToken(FILE *fp, char *line)
{
	char *p;

	for (p = line; *p; p++)
	{
		writeChar(fp, *p);
	}
}
void writeLine(FILE *fp, char *line)
{
	writeToken(fp, line);
	writeChar(fp, '\n');
}
void writeLines2Stream(FILE *fp, autoList_t *lines)
{
	char *line;
	uint index;

	foreach (lines, line, index)
	{
		writeLine(fp, line);
	}
}
void writeLines(char *file, autoList_t *lines)
{
	FILE *fp = fileOpen(file, "wt");

	writeLines2Stream(fp, lines);
	fileClose(fp);
}
void writeLines_b(char *file, autoList_t *lines)
{
	FILE *fp = fileOpen(file, "wb");

	writeLines2Stream(fp, lines);
	fileClose(fp);
}
void addLines2File(char *file, autoList_t *lines)
{
	FILE *fp = fileOpen(file, "at");

	writeLines2Stream(fp, lines);
	fileClose(fp);
}

char *readFirstLine(char *file)
{
	FILE *fp = fileOpen(file, "rt");
	char *line;

	line = readLine(fp);
	fileClose(fp);

	if (!line)
		line = strx("");

	return line;
}
void writeOneLine(char *file, char *line)
{
	FILE *fp = fileOpen(file, "wt");

	writeLine(fp, line);
	fileClose(fp);
}
void writeOneLineNoRet(char *file, char *line)
{
	FILE *fp = fileOpen(file, "wt");

	writeToken(fp, line);
	fileClose(fp);
}
void writeOneLineNoRet_b(char *file, char *line)
{
	FILE *fp = fileOpen(file, "wb");

	writeToken(fp, line);
	fileClose(fp);
}
void addLine2File(char *file, char *line)
{
	FILE *fp = fileOpen(file, "at");

	writeLine(fp, line);
	fileClose(fp);
}
void addLine2FileNoRet(char *file, char *line)
{
	FILE *fp = fileOpen(file, "at");

	writeToken(fp, line);
	fileClose(fp);
}
void addLine2FileNoRet_b(char *file, char *line)
{
	FILE *fp = fileOpen(file, "ab");

	writeToken(fp, line);
	fileClose(fp);
}

/*
	Little Endian
*/
void writeValue64Width(FILE *fp, uint64 value, uint width)
{
	uint bcnt;

	errorCase(!width);
	errorCase(sizeof(uint64) < width);

	for (bcnt = 0; bcnt < width; bcnt++)
	{
		writeChar(fp, value >> bcnt * 8 & 0xff);
	}
}
void writeValue64(FILE *fp, uint64 value)
{
	writeValue64Width(fp, value, sizeof(uint64));
}
void writeValueWidth(FILE *fp, uint value, uint width)
{
	writeValue64Width(fp, (uint64)value, width);
}
void writeValue(FILE *fp, uint value)
{
	writeValueWidth(fp, value, sizeof(uint));
}
uint64 readValue64Width(FILE *fp, uint width)
{
	uint64 value = 0;
	uint bcnt;

	for (bcnt = 0; bcnt < width; bcnt++)
	{
		int chr = readChar(fp);

		if (chr == EOF)
			break;

		value |= (uint64)chr << bcnt * 8;
	}
	return value;
}
uint64 readValue64(FILE *fp)
{
	return readValue64Width(fp, sizeof(uint64));
}
uint readValueWidth(FILE *fp, uint width)
{
	return (uint)readValue64Width(fp, width);
}
uint readValue(FILE *fp)
{
	return readValueWidth(fp, sizeof(uint));
}

uint readFirstValue(char *file)
{
	FILE *fp = fileOpen(file, "rb");
	uint value;

	value = readValue(fp);
	fileClose(fp);
	return value;
}
void writeOneValue(char *file, uint value)
{
	FILE *fp = fileOpen(file, "wb");

	writeValue(fp, value);
	fileClose(fp);
}

uint64 readFirstValue64(char *file)
{
	FILE *fp = fileOpen(file, "rb");
	uint value;

	value = readValue64(fp);
	fileClose(fp);
	return value;
}
void writeOneValue64(char *file, uint64 value)
{
	FILE *fp = fileOpen(file, "wb");

	writeValue64(fp, value);
	fileClose(fp);
}

BUFF *buffBind(FILE *fp, uint buffSize) // fp's mode == "rb" only
{
	BUFF *bp = (BUFF *)memAlloc(sizeof(BUFF));

	errorCase(buffSize < 1);

	bp->Fp = fp;
	bp->Buffer = NULL;
	bp->BuffSize = buffSize;
	return bp;
}
FILE *buffUnbind(BUFF *bp)
{
	FILE *fp = bp->Fp;

	if (bp->Buffer)
		releaseAutoBlock(bp->Buffer);

	memFree(bp);
	return fp;
}
BUFF *buffOpen(char *file, char *mode, uint buffSize) // mode == "rb" only
{
	errorCase(strcmp(mode, "rb"));
	return buffBind(fileOpen(file, mode), buffSize);
}
void buffClose(BUFF *bp)
{
	fileClose(buffUnbind(bp));
}
int buffReadChar(BUFF *bp)
{
	if (!bp->Buffer)
		goto read1storeof;

	if (bp->Index == getSize(bp->Buffer))
	{
		releaseAutoBlock(bp->Buffer);

	read1storeof:
		bp->Buffer = readBinaryStream(bp->Fp, bp->BuffSize);

		if (!bp->Buffer)
			return EOF;

		bp->Index = 0;
	}
	return getByte(bp->Buffer, bp->Index++);
}
void buffUnreadChar(BUFF *bp, int chr)
{
	if (chr == EOF)
		return;

	if (!bp->Buffer)
		bp->Buffer = newBlock();

	insertByte(bp->Buffer, 0, chr);
}
void buffUnreadBuffer(BUFF *bp)
{
	if (!bp->Buffer)
		return;

	fileSeek(bp->Fp, SEEK_CUR, -(sint64)(getSize(bp->Buffer) - bp->Index));
	bp->Index = getSize(bp->Buffer);
}

// c_
char *c_nnReadLine(FILE *fp)
{
	static char *stock;
	memFree(stock);
	return stock = nnReadLine(fp);
}
char *c_neReadLine(FILE *fp)
{
	static char *stock;
	memFree(stock);
	return stock = neReadLine(fp);
}

// _x
FILE *fileOpen_cx(char *file, char *mode)
{
	FILE *out = fileOpen(file, mode);
	memFree(mode);
	return out;
}
FILE *fileOpen_xc(char *file, char *mode)
{
	FILE *out = fileOpen(file, mode);
	memFree(file);
	return out;
}
FILE *fileOpen_xx(char *file, char *mode)
{
	FILE *out = fileOpen(file, mode);
	memFree(file);
	memFree(mode);
	return out;
}
autoList_t *readLines_x(char *file)
{
	autoList_t *out = readLines(file);
	memFree(file);
	return out;
}
char *readText_x(char *file)
{
	char *out = readText(file);
	memFree(file);
	return out;
}
void writeToken_x(FILE *fp, char *line)
{
	writeToken(fp, line);
	memFree(line);
}
void writeLine_x(FILE *fp, char *line)
{
	writeLine(fp, line);
	memFree(line);
}
void writeOneLine_cx(char *file, char *line)
{
	writeOneLine(file, line);
	memFree(line);
}
void writeOneLine_xc(char *file, char *line)
{
	writeOneLine(file, line);
	memFree(file);
}
void writeOneLine_xx(char *file, char *line)
{
	writeOneLine(file, line);
	memFree(file);
	memFree(line);
}
void addLine2File_cx(char *file, char *line)
{
	addLine2File(file, line);
	memFree(line);
}
void writeLines2Stream_x(FILE *fp, autoList_t *lines)
{
	writeLines2Stream(fp, lines);
	releaseDim(lines, 1);
}
void writeLines_cx(char *file, autoList_t *lines)
{
	writeLines(file, lines);
	releaseDim(lines, 1);
}
void writeLines_xc(char *file, autoList_t *lines)
{
	writeLines(file, lines);
	memFree(file);
}
void writeLines_xx(char *file, autoList_t *lines)
{
	writeLines_cx(file, lines);
	memFree(file);
}
void writeLines_b_cx(char *file, autoList_t *lines)
{
	writeLines_b(file, lines);
	releaseDim(lines, 1);
}
void addLines2File_cx(char *file, autoList_t *lines)
{
	addLines2File(file, lines);
	releaseDim(lines, 1);
}
void writeBinaryBlock_x(FILE *fp, autoBlock_t *block)
{
	writeBinaryBlock(fp, block);
	releaseAutoBlock(block);
}
void writeBinary_cx(char *file, autoBlock_t *block)
{
	writeBinary(file, block);
	releaseAutoBlock(block);
}
void writeBinary_xx(char *file, autoBlock_t *block)
{
	writeBinary_cx(file, block);
	memFree(file);
}
void writeJoinBinary_cx(char *file, autoBlock_t *block)
{
	writeJoinBinary(file, block);
	releaseAutoBlock(block);
}
void writeOneLineNoRet_cx(char *file, char *line)
{
	writeOneLineNoRet(file, line);
	memFree(line);
}
void writeOneLineNoRet_b_cx(char *file, char *line)
{
	writeOneLineNoRet_b(file, line);
	memFree(line);
}
void writeOneLineNoRet_b_xc(char *file, char *line)
{
	writeOneLineNoRet_b(file, line);
	memFree(file);
}
void writeOneLineNoRet_b_xx(char *file, char *line)
{
	writeOneLineNoRet_b_cx(file, line);
	memFree(file);
}
