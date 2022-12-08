/*
	T2DivId.exe の出力ファイルを、リクエスト・レスポンス毎にファイルに分ける。

	- - -

	T2DiHTTP.exe [入力ディレクトリ]
*/

#include "C:\Factory\Common\all.h"

static uint64 *MkUI64(uint64 value)
{
	uint64 *p = (uint64 *)memAlloc(sizeof(uint64));

	*p = value;
	return p;
}
static char *MkTmpFile(autoBlock_t *fileData)
{
	char *file = makeTempPath(NULL);

	writeBinary(file, fileData);
	return file;
}

// ---- ReadLine ----

static uint64 Stamp;
static uint Id;
static int Kind;
static autoBlock_t *RawData;

static int ReadLine(FILE *rfp)
{
	char *line = readLine(rfp);
	char *p;

	if (!line)
		return 0;

	errorCase(!lineExp("<17,09>,<4,10,09>,<1,CDRS>,\"<>\"", line));

	p = line + 17;
	p[0] = '\0';
	Stamp = toValue64(line);
	p[0] = ',';

	p = strchr(line + 22, ',');
	p[0] = '\0';
	Id = toValue(line + 18);
	p[0] = ',';

	Kind = p[1];

	if (RawData)
		releaseAutoBlock(RawData);

	RawData = newBlock();

	for (p += 4; *p != '"'; p++)
	{
		int chr;

		errorCase(!*p);

		if (*p == '\\' && p[1] && p[2])
		{
			int v1 = m_c2i(p[1]);
			int v2 = m_c2i(p[2]);

			chr = (v1 << 4) | v2;

			p += 2;
		}
		else
		{
			chr = *p;
		}
		addByte(RawData, chr);
	}
	return 1;
}

// ---- LoadFile ----

static autoList_t *SendStamps; // { uint64 *stamp, ... }
static autoList_t *SendFiles;
static autoList_t *RecvStamps; // { uint64 *stamp, ... }
static autoList_t *RecvFiles;
static uint ConId;

static void UnloadFile(void)
{
	char *file;
	uint index;

	if (!SendStamps) // ? ! loaded
		return;

	foreach (SendFiles, file, index)
		removeFile(file);

	foreach (RecvFiles, file, index)
		removeFile(file);

	releaseDim(SendStamps, 1);
	releaseDim(SendFiles, 1);
	releaseDim(RecvStamps, 1);
	releaseDim(RecvFiles, 1);
}
static void LoadFile(char *rFile)
{
	FILE *rfp = fileOpen(rFile, "rt");

	UnloadFile();

	SendStamps = newList();
	SendFiles = newList();
	RecvStamps = newList();
	RecvFiles = newList();
	ConId = 0;

	while (ReadLine(rfp))
	{
		errorCase(Id < 1000);

		if (ConId)
			errorCase(ConId != Id);
		else
			ConId = Id;

		switch (Kind)
		{
		case 'S':
			addElement(SendStamps, (uint)MkUI64(Stamp));
			addElement(SendFiles, (uint)MkTmpFile(RawData));
			break;

		case 'R':
			addElement(RecvStamps, (uint)MkUI64(Stamp));
			addElement(RecvFiles, (uint)MkTmpFile(RawData));
			break;

		case 'C':
		case 'D':
			break;

		default:
			error();
		}
	}
	fileClose(rfp);
}

// ---- RawDataReader ----

static autoList_t *R_Stamps;
static autoList_t *R_Files;
static uint R_Index;
static FILE *R_Fp;

static void ResetRawDataReader(autoList_t *stamps, autoList_t *files)
{
	R_Stamps = stamps;
	R_Files = files;
	R_Index = 0;
	R_Fp = NULL;
}
static int R_ReadChar(void)
{
	int chr;

	for (; ; )
	{
		if (!R_Fp)
		{
			if (getCount(R_Files) <= R_Index)
				return EOF;

			R_Fp = fileOpen(getLine(R_Files, R_Index), "rb");
		}
		chr = readChar(R_Fp);

		if (chr != EOF)
			break;

		fileClose(R_Fp);
		R_Index++;
		R_Fp = NULL;
	}
	return chr;
}
static uint64 R_GetStamp(void)
{
	if (R_Index < getCount(R_Stamps))
	{
		return *(uint64 *)getElement(R_Stamps, R_Index);
	}
	if (getCount(R_Stamps))
	{
		return *(uint64 *)getLastElement(R_Stamps);
	}
	return IMAX_64 - 1;
}

// ---- RawDataWriter ----

static uint W_Index;
static char *W_Direction;

static void ResetRawDataWriter(char *direction)
{
	W_Index = 1000;
	W_Direction = direction;
}

#define W_LINELENMAX 65000

static char *W_File;
static FILE *W_Fp;
static int W_Wrote;
static uint64 W_StartStamp;

static void W_Start(void)
{
	W_File = makeTempPath(NULL);
	W_Fp = fileOpen(W_File, "wb");
	W_Wrote = 0;
	W_StartStamp = 0;
}
static int W_ReadChar(void)
{
	int chr = R_ReadChar();

	if (chr != EOF)
	{
		writeChar(W_Fp, chr);

		if (!W_Wrote)
		{
			W_Wrote = 1;
			W_StartStamp = R_GetStamp();
		}
	}
	return chr;
}
static char *W_ReadLine(void)
{
	autoBlock_t *buff = newBlock();

	for (; ; )
	{
		int chr = W_ReadChar();

		if (chr == EOF)
		{
			if (!getSize(buff))
			{
				releaseAutoBlock(buff);
				return NULL;
			}
			break;
		}
		if (chr == '\r')
			continue;

		if (chr == '\n')
			break;

		if (W_LINELENMAX <= getSize(buff))
		{
			cout("Warning: over W_LINELENMAX\n");
			break;
		}
		addByte(buff, chr);
	}
	return unbindBlock2Line(buff);
}
static void W_Destroy(void)
{
	fileClose(W_Fp);
	removeFile(W_File);

	memFree(W_File);
}
static void W_End_File(char *name)
{
	char *destFile = getOutFile(name);

	fileClose(W_Fp);
	moveFile(W_File, destFile);

	memFree(W_File);
	memFree(destFile);
}
static void W_End(void)
{
	uint64 stamp1 = W_StartStamp;
	uint64 stamp2 = R_GetStamp();
	uint id = ConId;
	uint index = W_Index;
	char *direction = W_Direction;
	char *name;

	name = xcout("%I64u_%I64u_%u_%u_%s.txt", stamp1, stamp2, id, index, direction);
	W_End_File(name);
	memFree(name);

	W_Index++;
}

// ---- PraseHTTPReqRes ----

static void PHRR_Parse(void)
{
	uint64 contentLength = 0;
	int chunked = 0;

	for (; ; )
	{
		char *line = W_ReadLine();
		char *p;
		char *name;
		char *value;

		if (!line)
			break;

		if (!*line)
		{
			memFree(line);
			break;
		}
		p = strchr(line, ':');

		if (p)
		{
			*p = '\0';
			name = strx(line);
			value = strx(p + 1);

			ucTrimEdge(name);
			ucTrimEdge(value);

			if (!_stricmp(name, "Content-Length"))
			{
				contentLength = toValue64(value);
			}
			else if (!_stricmp(name, "Transfer-Encoding") && !_stricmp(value, "chunked"))
			{
				chunked = 1;
			}
			memFree(name);
			memFree(value);
		}
		memFree(line);
	}
	if (chunked)
	{
		for (; ; )
		{
			char *line = W_ReadLine();
			uint size;
			uint count;

			if (!line)
				break;

			strchrEnd(line, ';')[0] = '\0'; // chunk-extension 削除
			size = toValueDigits(line, hexadecimal);
			memFree(line);

			if (!size)
				break;

			for (count = 0; count < size; count++)
				if (W_ReadChar() == EOF)
					break;

			W_ReadChar(); // CR
			W_ReadChar(); // LF
		}
		for (; ; ) // chunk-trailer 読み飛ばし
		{
			char *line = W_ReadLine();

			if (!line)
				break;

			if (!*line)
			{
				memFree(line);
				break;
			}
			memFree(line);
		}
	}
	else
	{
		uint64 count;

		for (count = 0; count < contentLength; count++)
			if (W_ReadChar() == EOF)
				break;
	}
}
static void PHRR_Main(char *direction)
{
	ResetRawDataWriter(direction);

	for (; ; )
	{
		W_Start();

		PHRR_Parse();

		if (!W_Wrote)
		{
			W_Destroy();
			break;
		}
		W_End();
	}
}
static void ParseHTTPReqRes(void)
{
	ResetRawDataReader(SendStamps, SendFiles);
	PHRR_Main("1-Send");
	ResetRawDataReader(RecvStamps, RecvFiles);
	PHRR_Main("2-Recv");
}

// ----

static void ProcFile(char *rFile)
{
	LoadFile(rFile);
	ParseHTTPReqRes();
}
static void T2DiHTTP(char *dir)
{
	autoList_t *files = lsFiles(dir);
	char *file;
	uint index;

	foreach (files, file, index)
	{
		ProcFile(file);
	}
	releaseDim(files, 1);
}
int main(int argc, char **argv)
{
	T2DiHTTP(hasArgs(1) ? nextArg() : c_dropDir());
	openOutDir();

	UnloadFile();
}
