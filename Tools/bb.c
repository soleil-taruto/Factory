/*
	bb.exe [/A] /C

		�h���b�v����2�̃t�@�C�����r����B

	bb.exe [/A] /E [�t�@�C��]

		�t�@�C�����{���E�ҏW����B�G�f�B�^�ŊJ���B

	bb.exe [/A] /P

		�G�f�B�^���J���ē\��t����������̃o�C�i����W���o�͂���B

	bb.exe (/SCT | /B2T | /B2H) ���̓t�@�C�� [�o�̓t�@�C��]

		���̓t�@�C���̃o�C�i�����u�V���v���ȁv16�i���e�L�X�g�Ƃ��ďo�͂���B

	bb.exe (/SCT | /T2B | /H2B) ���̓t�@�C�� �o�̓t�@�C��

		���͂��ꂽ16�i���e�L�X�g���o�C�i���Ƃ��ďo�͂���B
		���͂́A�A�X�L�[���� 0123456789abcdefABCDEF �ȊO�𖳎����āA2�������ɘA�����ăo�C�g�l�ɂ���B

	bb.exe ���̓t�@�C�� �J�n�ʒu �o�C�g��

		���̓t�@�C���́u�J�n�ʒu�v����u�o�C�g���v����16�i���e�L�X�g�Ƃ��ĕW���o�͂���B
		�J�n�ʒu���t�@�C���̏I�[�𒴂���ꍇ�A�����o�͂��Ȃ��B
		�o�C�g���� 0 ���A�t�@�C���̏I�[�𒴂���ꍇ�A�t�@�C���̏I�[�܂ŁB

	bb.exe /RNG ���̓t�@�C�� �J�n�ʒu �o�C�g�� �o�̓t�@�C��

		���̓t�@�C���́u�J�n�ʒu�v����u�o�C�g���v�����o�̓t�@�C���ɏo�͂���B
		�J�n�ʒu���t�@�C���̏I�[�𒴂���ꍇ�A�����o�͂��Ȃ��B
		�o�C�g���� 0 ���A�t�@�C���̏I�[�𒴂���ꍇ�A�t�@�C���̏I�[�܂ŁB

	bb.exe /MID ���̓t�@�C�� �J�n�ʒu �o�C�g�� [�o�̓t�@�C��]

		���̓t�@�C���́u�J�n�ʒu�v����u�o�C�g���v�����u�W���o�́b�o�̓t�@�C���v�ɏo�͂���B
		�J�n�ʒu���t�@�C���̏I�[�𒴂���ꍇ�A/TAIL �Ɠ����B
		�o�C�g���� 0 ���A�t�@�C���̏I�[�𒴂���ꍇ�A�t�@�C���̏I�[�܂ŁB

	bb.exe /HEAD ���̓t�@�C�� �o�C�g�� [�o�̓t�@�C��]

		���̓t�@�C���̐擪����u�o�C�g���v�����u�W���o�́b�o�̓t�@�C���v�ɏo�͂���B
		�o�C�g�����t�@�C���T�C�Y���傫���ꍇ�A�t�@�C���S�́B

	bb.exe /TAIL ���̓t�@�C�� �o�C�g�� [�o�̓t�@�C��]

		���̓t�@�C���̏I�[����u�o�C�g���v�����u�W���o�́b�o�̓t�@�C���v�ɏo�͂���B
		�o�C�g�����t�@�C���T�C�Y���傫���ꍇ�A�t�@�C���S�́B

	bb.exe [/A] ���̓t�@�C��1 ���̓t�@�C��2

		2�̓��̓t�@�C�����r����B

	bb.exe [/A] ���̓t�@�C��

		���̓t�@�C���̃o�C�i����16�i���e�L�X�g�Ƃ��ĕW���o�͂���B

	bb.exe [/A] /D

		�h���b�v�����t�@�C���̃o�C�i����16�i���e�L�X�g�Ƃ��ĕW���o�͂���B

	bb.exe [/A]

		�W�����͂̃o�C�i����16�i���e�L�X�g�Ƃ��ĕW���o�͂���B
*/

#include "C:\Factory\Common\all.h"

// ---- Bin_To_Text ----

#define BYTE_PER_LINE 16
#define TEXT_SEP_POS 61
#define TEXT_SEP_CHAR '|'

static int AppendTextMode;

static autoList_t *BinToText(autoBlock_t *block)
{
	autoList_t *lines = newList();
	uint index;
	char text[BYTE_PER_LINE + 1];

	for (index = 0; index < getSize(block); index += BYTE_PER_LINE)
	{
		char *line = xcout("%08x", index);
		uint bidx;
		char *text_p = text;

		for (bidx = 0; bidx < BYTE_PER_LINE && index + bidx < getSize(block); bidx++)
		{
			int chr = getByte(block, index + bidx);

			if (bidx == 0 || bidx == BYTE_PER_LINE / 2)
				line = addChar(line, ' ');

			line = addLine_x(line, xcout(" %02x", chr));

			if (chr == '\0')
				chr = '_';

			*text_p++ = chr;
		}
		if (AppendTextMode)
		{
			*text_p = '\0';
			line2JLine(text, 1, 0, 0, 1);

			while (strlen(line) < TEXT_SEP_POS)
				line = addChar(line, ' ');

			line = addChar(line, TEXT_SEP_CHAR);
			line = addChar(line, ' ');
			line = addLine(line, text);
		}
		addElement(lines, (uint)line);
	}
	return lines;
}

static uint TTB_HasonCount;

static autoBlock_t *TextToBin(autoList_t *lines)
{
	autoBlock_t *block = newBlock();
	char *line;
	uint index;

	TTB_HasonCount = 0;

	foreach (lines, line, index)
	{
		line = strx(line);
		strchrEnd(line, TEXT_SEP_CHAR)[0] = '\0'; // @ AppendTextMode
		removeChar(line, ' ');

		if (!lineExp("<09afAF>", line) || strlen(line) < 8 || strlen(line) % 2 != 0)
		{
			cout("�j�� [%u]: %s\n", index + 1, line);
			TTB_HasonCount++;
		}
		ab_addBytes_x(block, makeBlockHexLine(line + m_min(8, strlen(line))));
		memFree(line);
	}
	return block;
}

// ----

static void CompareBinFile(char *file1, char *file2)
{
	autoBlock_t *block1 = readBinary(file1);
	autoBlock_t *block2 = readBinary(file2);
	autoList_t *lines1;
	autoList_t *lines2;
	autoList_t *report;
	char *line;
	uint index;

	lines1 = BinToText(block1);
	lines2 = BinToText(block2);

	releaseAutoBlock(block1);
	releaseAutoBlock(block2);

	report = getDiffLinesReport(lines1, lines2, 0);

	releaseDim(lines1, 1);
	releaseDim(lines2, 1);

	if (getCount(report))
	{
		cout("+--------------------+\n");
		cout("| ��v���܂���ł��� |\n");
		cout("+--------------------+\n");

		foreach (report, line, index)
		{
			cout("%s\n", line);
		}
	}
	else
	{
		cout("+--------------+\n");
		cout("| ��v���܂��� |\n");
		cout("+--------------+\n");
	}
	releaseDim(report, 1);
}
static void EditBinFile(char *file)
{
	autoBlock_t *block = readBinary(file);
	autoBlock_t *edBlock;
	autoList_t *lines;
	autoList_t *edLines;

	lines = BinToText(block);
reedit:
	edLines = editLines(lines);
	edBlock = TextToBin(edLines);

	if (TTB_HasonCount)
	{
		cout("�ǂ����j�����Ă���悤�ł��B\n");
		cout("�ĕҏW�H\n");

		if (clearGetKey() != 0x1b)
		{
			releaseAutoBlock(edBlock);
			releaseDim(lines, 1);
			lines = edLines;
			goto reedit;
		}
	}
	if (!isSameBlock(block, edBlock))
	{
		cout("�ǂ����C�����ꂽ�悤�ł��B\n");
		cout("�㏑���ۑ��H\n");

		if (clearGetKey() != 0x1b)
		{
			cout("�������ݒ�...\n");
			writeBinary(file, edBlock);
			cout("�������݂܂����B\n");
		}
	}
	releaseAutoBlock(block);
	releaseAutoBlock(edBlock);
	releaseDim(lines, 1);
	releaseDim(edLines, 1);
}
static void DumpBinFile(char *file) // file == NULL �̂Ƃ��� stdin ����
{
	autoBlock_t *block = file ? readBinary(file) : readBinaryToEnd(stdin, NULL);
	autoList_t *lines;
	char *line;
	uint index;

	lines = BinToText(block);

	foreach (lines, line, index)
		cout("%s\n", line);

	releaseAutoBlock(block);
	releaseDim(lines, 1);
}

static void SimpleConvToText(char *file, char *outFile)
{
	FILE *fp = fileOpen(file, "rb");
	FILE *outFp = outFile ? fileOpen(outFile, "wt") : stdout;

	for (; ; )
	{
		int chr = readChar(fp);

		if (chr == EOF)
			break;

		writeToken_x(outFp, xcout("%02x", chr));
	}
	fileClose(fp);

	if (outFile)
		fileClose(outFp);
}
static void SimpleConvToBinary(char *file, char *outFile)
{
	FILE *fp = fileOpen(file, "rt");
	FILE *outFp = fileOpen(outFile, "wb");
	int stockLead = 0;
	uint stockValue;

	for (; ; )
	{
		int chr = readChar(fp);

		if (chr == EOF)
			break;

		if (m_ishexadecimal(chr))
		{
			uint value = m_c2i(chr);

			if (stockLead)
				writeChar(outFp, stockValue << 4 | value);
			else
				stockValue = value;

			stockLead = !stockLead;
		}
	}
	fileClose(fp);
	fileClose(outFp);
}
static void DoMid(char *rFile, uint64 start, uint64 size, char *wFile) // wFile: NULL == stdout
{
	uint64 rFileSize = getFileSize(rFile);
	uint64 count;
	FILE *rfp;
	FILE *wfp;

	if (rFileSize < start)
	{
		m_minim(size, rFileSize);
		start = rFileSize - size;
	}
	else
	{
		m_minim(size, rFileSize - start);
	}

	rfp = fileOpen(rFile, "rb");
	fileSeek(rfp, SEEK_SET, start);

	if (wFile)
	{
		cout("< %s\n", rFile);
		cout("# %I64u, %I64u\n", start, size);
		cout("> %s\n", wFile);

		wfp = fileOpen(wFile, "wb");

		for (count = 0; count < size; count++)
		{
			writeChar(wfp, readChar(rfp));
		}
		fileClose(wfp);
	}
	else
	{
		for (count = 0; count < size; count++)
		{
			int chr = readChar(rfp);
			int lf;
			int ret;

			lf = chr == '\n';
			m_toHalf(chr);

			ret = putc(chr, stdout); // macro
			errorCase(ret != chr);

			if (lf)
			{
				ret = putc('\n', stdout); // macro
				errorCase(ret != '\n');
			}
		}
	}
	fileClose(rfp);
}

int main(int argc, char **argv)
{
	stdin_set_bin();

readArgs:
	if (argIs("/A"))
	{
		AppendTextMode = 1;
		goto readArgs;
	}

	if (argIs("/C")) // drop and Compare
	{
		char *file1;
		char *file2;

		for (; ; )
		{
			file1 = dropFile();
			file2 = dropFile();

			CompareBinFile(file1, file2);
			cout("\n");

			memFree(file1);
			memFree(file2);
		}
		error(); // dummy
	}
	if (argIs("/E")) // Edit
	{
		char *file;

		if (hasArgs(1))
			file = nextArg();
		else
			file = c_dropFile();

		EditBinFile(file);
		return;
	}
	if (argIs("/P")) // copy and Paste
	{
		char *file = makeTempFile("txt");

		editTextFile(file);
		DumpBinFile(file);
		removeFile(file);
		memFree(file);
		return;
	}
	if (argIs("/SCT") || argIs("/B2T") || argIs("/B2H")) // Simple Conversion (to Text), Binary To Hex-Text
	{
		SimpleConvToText(getArg(0), hasArgs(2) ? getArg(1) : NULL);
		return;
	}
	if (argIs("/SCB") || argIs("/T2B") || argIs("/H2B")) // Simple Conversion (to Binary), Hex-Text To Binary
	{
		SimpleConvToBinary(getArg(0), getArg(1));
		return;
	}
	if (argIs("/RNG"))
	{
		char *rFile;
		char *wFile;
		uint64 start;
		uint64 size;
		FILE *rfp;
		FILE *wfp;
		uint64 count;

		rFile = nextArg();
		start = toValue64(nextArg());
		size = toValue64(nextArg());
		wFile = nextArg();

		cout("< %s\n", rFile);
		cout("# %I64u, %I64u\n", start, size);
		cout("> %s\n", wFile);

		errorCase(SINT64MAX < start);

		if (!size)
			size = UINT64MAX;

		rfp = fileOpen(rFile, "rb");
		wfp = fileOpen(wFile, "wb");

		fileSeek(rfp, SEEK_SET, start);

		for (count = 0; count < size; count++)
		{
			int chr = readChar(rfp);

			if (chr == EOF)
				break;

			writeChar(wfp, chr);
		}
		fileClose(rfp);
		fileClose(wfp);
		return;
	}
	if (argIs("/MID"))
	{
		char *rFile;
		char *wFile = NULL;
		uint64 start;
		uint64 size;

		rFile = nextArg();
		start = toValue64(nextArg());
		size = toValue64(nextArg());

		if (hasArgs(1))
			wFile = nextArg();

		DoMid(rFile, start, size, wFile);
		return;
	}
	if (argIs("/HEAD"))
	{
		char *rFile;
		char *wFile = NULL;
		uint64 size;

		rFile = nextArg();
		size = toValue64(nextArg());

		if (hasArgs(1))
			wFile = nextArg();

		DoMid(rFile, 0, size, wFile);
		return;
	}
	if (argIs("/TAIL"))
	{
		char *rFile;
		char *wFile = NULL;
		uint64 size;

		rFile = nextArg();
		size = toValue64(nextArg());

		if (hasArgs(1))
			wFile = nextArg();

		DoMid(rFile, UINT64MAX, size, wFile);
		return;
	}
	if (argIs("/D"))
	{
		DumpBinFile(c_dropFile());
		return;
	}
	if (hasArgs(3))
	{
		char *file = getArg(0);
		FILE *fp;
		uint64 start = toValue64(getArg(1));
		uint64 size = toValue64(getArg(2));
		uint64 count;

		errorCase(SINT64MAX < start);

		if (!size)
			size = UINT64MAX;

		fp = fileOpen(file, "rb");
		fileSeek(fp, SEEK_SET, start);

		for (count = 0; count < size; count++)
		{
			int chr = readChar(fp);

			if (chr == EOF)
				break;

			if (count)
				cout("%c", count % 16 ? ' ' : '\n');

			cout("%02x", chr);
		}
		if (count)
			cout("\n");

		fileClose(fp);
		return;
	}
	if (hasArgs(2))
	{
		char *file1 = getArg(0);
		char *file2 = getArg(1);

		CompareBinFile(file1, file2);
		return;
	}
	if (hasArgs(1))
	{
		DumpBinFile(nextArg());
		return;
	}
	DumpBinFile(NULL);
}
