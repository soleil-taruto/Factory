/*
	フォームはこんな感じ、
		<form method="post" accept-charset="Shift_JIS" action="http://localhost/" enctype="multipart/form-data">
		<input type="file" name="n-file"/>
		</form>

	一つの接続は、最悪で httpMultiPartContentLenMax バイト程度の受信データファイルを生成し、各パートに分解するとき一時的に更に同程度使用する。
	最悪のディスク使用量として httpMultiPartContentLenMax * (最大接続数 + 1) バイトくらいを考えること。

	添付ファイル以外の <input> も入れられる。
	こんな感じ、
		<form method="post" accept-charset="Shift_JIS" action="http://localhost/" enctype="multipart/form-data">
		<div>text: <input type="text" name="n-text"/></div>
		<div>file: <input type="file" name="n-file"/></div>
		<div><input type="checkbox" name="option-0" value="checked"/>check-box-0</div>
		<div><input type="checkbox" name="option-1" value="checked" checked/>check-box-1</div>
		<div><input type="submit" value="送信"></div>
		</form>

	それぞれの <input> はパートに分かれて上がってくる。
	順序がどうなるかは知らん。たぶん記述順
*/

#include "MultiPart.h"

uint httpMultiPartContentLenMax = 2100000000; // 2^31 あたりに壁がある？
int httpM4UServerMode;

#define CONTENT_BUFF_SIZE (1024 * 512)
#define BOUNDARY_LENMAX 1024 // 1～70文字らしい
#define PART_NUMMAX 1024
#define PARTLINE_LENMAX (1024 * 16)

static int RecvStream(SockStream_t *i, FILE *fp, uint recvSize) // ret: errored
{
	autoBlock_t *subBlock;
	uint subSize;

	while (recvSize)
	{
		subSize = m_min(recvSize, CONTENT_BUFF_SIZE);
		subBlock = nobCreateBlock(subSize);

		if (!SockRecvBlock(i, directGetBuffer(subBlock), subSize))
		{
			if (httpM4UServerMode)
			{
				setSize(subBlock, SockRecvBlock_LastRecvSize);
				writeBinaryBlock(fp, subBlock);
				recvSize -= SockRecvBlock_LastRecvSize;

				cout("M4UServer_Mode\n");
				cout("★★★ 最後まで受信できませんでしたが受信完了したことにします ★★★\n");
				cout("受信できなかったサイズ : %u バイト\n", recvSize);

				i->Extra.M4UServerMode_Operated = 1;

				releaseAutoBlock(subBlock);
				return 0;
			}
			releaseAutoBlock(subBlock);
			return 1;
		}
		writeBinaryBlock(fp, subBlock);
		recvSize -= subSize;

		releaseAutoBlock(subBlock);
	}
	return 0;
}
/*
	httpMultiPartContentLenMax を超える場合や、エラーの場合 contentFile を 0 バイトにする。
	このとき、受信を全くしないか中断する -> C 送信が終われない -> C 受信に入れない -> S 送信を終われない -> で、固まらないか少し不安
	DestroySockStream() するか、SetSockStreamTimeout() で応答用の短いタイムアウトを設定した方が良いかも？
*/
void httpRecvRequestFile(SockStream_t *i, char **pHeader, char *contentFile)
{
	FILE *conFp = fileOpen(contentFile, "wb");
	int chunked;
	uint cSize;
	uint nextCSize;
	int errored = 0;

	httpRecvRequestHeader(i, pHeader, &chunked, &cSize);

	if (chunked)
	{
		httpChunkedRecver_t *cr = httpCreateChunkedRecver(i);

		cSize = 0;

		for (; ; )
		{
			autoBlock_t *subBlock = httpRecvChunked(cr);

			if (!subBlock)
				break;

			if (httpMultiPartContentLenMax - cSize < getSize(subBlock)) // Overflow
			{
				releaseAutoBlock(subBlock);
				errored = 1;
				break;
			}
			writeBinaryBlock(conFp, subBlock);
			cSize += getSize(subBlock);
			releaseAutoBlock(subBlock);
		}
		httpReleaseChunkedRecver(cr);
	}
	else
	{
		if (httpMultiPartContentLenMax < cSize) // Overflow
			errored = 1;
		else
			errored = RecvStream(i, conFp, cSize);
	}
	fileClose(conFp);

	if (errored)
		createFile(contentFile);
}

static char *RNP_Boundary;
static char *RNP_Name;
static char *RNP_LocalFile;
static char *RNP_BodyFile;

static void RNP_ReadBoundary(FILE *conFp)
{
	autoBlock_t *boundary = newBlock();

	ab_addLine(boundary, "\r\n"); // CR-LF

	for (; ; )
	{
		int chr = readChar(conFp);

		if (chr == EOF) // Invalid EOF
			break;

		if (chr == '\r') // CR
		{
			readChar(conFp); // LF
			break;
		}
		if (BOUNDARY_LENMAX <= getSize(boundary)) // ? Overflow
			break;

		addByte(boundary, chr);
	}
	memFree(RNP_Boundary);
	RNP_Boundary = ab_makeLine_x(boundary);
}
static char *RNP_GetValue(char *line, char *leader)
{
	char *p = mbs_stristr(line, leader);

	if (!p)
		return strx("<未定義>");

	p[0] = '\0'; // 'filename=' を 'name=' として読まないように、、、
	p += strlen(leader);
	strchrEnd(p, '"')[0] = '\0';

	line2JLine(p, 1, 0, 0, 1);

	if (!*p)
		p = "<無効>"; // 空文字列の回避

	p = strx(p);
	return p;
}
static void RNP_ReadHeader(FILE *conFp)
{
	char *name      = NULL;
	char *localFile = NULL;

	for (; ; )
	{
		char *line = readLineLenMax(conFp, PARTLINE_LENMAX); // CR-LF まで読み込む

		if (!line) // Invalid EOF
			break;

		if (!*line)
		{
			memFree(line);
			break;
		}
		if (startsWithICase(line, "Content-Disposition:"))
		{
			memFree(name);
			memFree(localFile);
			localFile = RNP_GetValue(line, "filename=\"");
			name      = RNP_GetValue(line, "name=\"");
		}
		memFree(line);
	}
	if (!name)      name      = strx("<未検出>");
	if (!localFile) localFile = strx("<未検出>");

//	memFree(RNP_Name);
//	memFree(RNP_LocalFile);
	RNP_Name      = name;
	RNP_LocalFile = localFile;
}
static void ReadNextPart(FILE *conFp)
{
	char *file = makeTempPath("content-part");
	FILE *fp;
	BUFF *conBp = buffBind(conFp, CONTENT_BUFF_SIZE);
	uint bCount = 0;

	fp = fileOpen(file, "wb");

	for (; ; )
	{
		int chr = buffReadChar(conBp);

		if (chr == EOF) // Invalid EOF
			break;

	recheck:
		if (chr == RNP_Boundary[bCount])
		{
			bCount++;

			if (RNP_Boundary[bCount] == '\0')
			{
				break;
			}
		}
		else
		{
			if (bCount)
			{
				autoBlock_t gab;

				fileWrite(fp, gndBlockVar(RNP_Boundary, bCount, gab));
				bCount = 0;
				goto recheck; // RNP_Boundary[0] は [1] 以降に含まれていない事を前提とする。[0] は CR なので、含まれてることはまず無いだろう。
			}
			writeChar(fp, chr);
		}
	}
	fileClose(fp);
	buffUnreadBuffer(conBp);
	buffUnbind(conBp);

//	memFree(RNP_BodyFile);
	RNP_BodyFile = file;
}
static int RNP_ReadTrailer(FILE *conFp) // ret: ? 継続
{
	// 継続なら CR-LF, 終了なら "--" + CR-LF
	return
		readChar(conFp) == '\r' &&
		readChar(conFp) == '\n';
}

autoList_t *httpDivideContent(char *contentFile) // ret: 1 <= getCount(ret)
{
	FILE *fp = fileOpen(contentFile, "rb");
	autoList_t *parts = newList();

	RNP_ReadBoundary(fp);
	do
	{
		httpPart_t *part;

		if (PART_NUMMAX <= getCount(parts)) // ? Overflow
			break;

		RNP_ReadHeader(fp);
		ReadNextPart(fp);

		part = (httpPart_t *)memAlloc(sizeof(httpPart_t));
		part->Name      = RNP_Name;
		part->LocalFile = RNP_LocalFile;
		part->BodyFile  = RNP_BodyFile;

		addElement(parts, (uint)part);
	}
	while (RNP_ReadTrailer(fp));

	fileClose(fp);
	return parts;
}
void httpReleaseParts(autoList_t *parts)
{
	httpPart_t *part;
	uint index;

	foreach (parts, part, index)
	{
		removeFile(part->BodyFile);

		memFree(part->Name);
		memFree(part->LocalFile);
		memFree(part->BodyFile);
		memFree(part);
	}
	releaseAutoList(parts);
}
httpPart_t *httpGetPart(autoList_t *parts, char *name)
{
	httpPart_t *part;
	uint index;

	foreach (parts, part, index)
	{
		if (!_stricmp(part->Name, name))
		{
			return part;
		}
	}
	return (httpPart_t *)getElement(parts, 0); // 何でもいいから何処かのパートを返しちゃう。
}

autoList_t *httpRecvRequestMultiPart(SockStream_t *i, char **pHeader)
{
	char *conFile = makeTempPath("content-multi-part");
	autoList_t *parts;

	httpRecvRequestFile(i, pHeader, conFile);
	parts = httpDivideContent(conFile);

	removeFile(conFile);
	memFree(conFile);

	return parts;
}

char *httpGetPartLine(autoList_t *parts, char *name)
{
	httpPart_t *part = httpGetPart(parts, name);
	char *line;

	if (PARTLINE_LENMAX < getFileSize(part->BodyFile))
	{
		return strx("<サイズ上限超過>");
	}
	line = readFirstLine(part->BodyFile);
//	line2JLine(p, 1, 0, 0, 1);
	return line;
}
char *c_httpGetPartLine(autoList_t *parts, char *name)
{
	static char *stock;
	memFree(stock);
	return stock = httpGetPartLine(parts, name);
}
