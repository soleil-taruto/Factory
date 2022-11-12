/*
	�t�H�[���͂���Ȋ����A
		<form method="post" accept-charset="Shift_JIS" action="http://localhost/" enctype="multipart/form-data">
		<input type="file" name="n-file"/>
		</form>

	��̐ڑ��́A�ň��� httpMultiPartContentLenMax �o�C�g���x�̎�M�f�[�^�t�@�C���𐶐����A�e�p�[�g�ɕ�������Ƃ��ꎞ�I�ɍX�ɓ����x�g�p����B
	�ň��̃f�B�X�N�g�p�ʂƂ��� httpMultiPartContentLenMax * (�ő�ڑ��� + 1) �o�C�g���炢���l���邱�ƁB

	�Y�t�t�@�C���ȊO�� <input> ���������B
	����Ȋ����A
		<form method="post" accept-charset="Shift_JIS" action="http://localhost/" enctype="multipart/form-data">
		<div>text: <input type="text" name="n-text"/></div>
		<div>file: <input type="file" name="n-file"/></div>
		<div><input type="checkbox" name="option-0" value="checked"/>check-box-0</div>
		<div><input type="checkbox" name="option-1" value="checked" checked/>check-box-1</div>
		<div><input type="submit" value="���M"></div>
		</form>

	���ꂼ��� <input> �̓p�[�g�ɕ�����ďオ���Ă���B
	�������ǂ��Ȃ邩�͒m���B���Ԃ�L�q��
*/

#include "MultiPart.h"

uint httpMultiPartContentLenMax = 2100000000; // 2^31 ������ɕǂ�����H
int httpM4UServerMode;

#define CONTENT_BUFF_SIZE (1024 * 512)
#define BOUNDARY_LENMAX 1024 // 1�`70�����炵��
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
				cout("������ �Ō�܂Ŏ�M�ł��܂���ł�������M�����������Ƃɂ��܂� ������\n");
				cout("��M�ł��Ȃ������T�C�Y : %u �o�C�g\n", recvSize);

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
	httpMultiPartContentLenMax �𒴂���ꍇ��A�G���[�̏ꍇ contentFile �� 0 �o�C�g�ɂ���B
	���̂Ƃ��A��M��S�����Ȃ������f���� -> C ���M���I���Ȃ� -> C ��M�ɓ���Ȃ� -> S ���M���I���Ȃ� -> �ŁA�ł܂�Ȃ��������s��
	DestroySockStream() ���邩�ASetSockStreamTimeout() �ŉ����p�̒Z���^�C���A�E�g��ݒ肵�������ǂ������H
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
		return strx("<����`>");

	p[0] = '\0'; // 'filename=' �� 'name=' �Ƃ��ēǂ܂Ȃ��悤�ɁA�A�A
	p += strlen(leader);
	strchrEnd(p, '"')[0] = '\0';

	line2JLine(p, 1, 0, 0, 1);

	if (!*p)
		p = "<����>"; // �󕶎���̉��

	p = strx(p);
	return p;
}
static void RNP_ReadHeader(FILE *conFp)
{
	char *name      = NULL;
	char *localFile = NULL;

	for (; ; )
	{
		char *line = readLineLenMax(conFp, PARTLINE_LENMAX); // CR-LF �܂œǂݍ���

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
	if (!name)      name      = strx("<�����o>");
	if (!localFile) localFile = strx("<�����o>");

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
				goto recheck; // RNP_Boundary[0] �� [1] �ȍ~�Ɋ܂܂�Ă��Ȃ�����O��Ƃ���B[0] �� CR �Ȃ̂ŁA�܂܂�Ă邱�Ƃ͂܂��������낤�B
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
static int RNP_ReadTrailer(FILE *conFp) // ret: ? �p��
{
	// �p���Ȃ� CR-LF, �I���Ȃ� "--" + CR-LF
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
	return (httpPart_t *)getElement(parts, 0); // ���ł��������牽�����̃p�[�g��Ԃ����Ⴄ�B
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
		return strx("<�T�C�Y�������>");
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
