/*
	HTTP のマルチパート body を作成する。

	fileStore, Post/Uploader の場合、ヘッダの Content-Type に boundary を書かなくても良い。
	<- マルチパートであること前提なので body の先頭部を { "--" + boundary + CRLF } と見なして、
	ここから boundary を読み取るため。
*/

#include "MakeMultiPartContent.h"

#define CRLF "\r\n"

static autoList_t *TextList;
static autoList_t *FileNameList;
static autoList_t *BodyList;

void mmpc_addPart_xxx(char *text, char *fileName, autoBlock_t *body)
{
	errorCase(!text);
	errorCase(!fileName);
	errorCase(!body);

	if (!TextList)
	{
		TextList = newList();
		FileNameList = newList();
		BodyList = newList();
	}
	addElement(TextList, (uint)text);
	addElement(FileNameList, (uint)fileName);
	addElement(BodyList, (uint)body);
}
void mmpc_addPart(char *text, char *fileName, autoBlock_t *body)
{
	errorCase(!text);
	errorCase(!fileName);
	errorCase(!body);

	mmpc_addPart_xxx(strx(text), strx(fileName), copyAutoBlock(body));
}
void mmpc_addPartFile(char *text, char *fileName, char *bodyFile)
{
	errorCase(!text);
	errorCase(!fileName);
	errorCase(!bodyFile);
	errorCase(!existFile(bodyFile));

	mmpc_addPart_xxx(strx(text), strx(fileName), readBinary(bodyFile));
}

static int IsIncludeBoundary(char *boundary) // boundary: [0] は [1] 以降に存在しないこと。
{
	autoBlock_t *body;
	uint body_index;

	foreach (BodyList, body, body_index)
	{
		char *p_bdr = boundary;
		uint index;

		for (index = 0; index < getSize(body); index++)
		{
			int chr = getByte(body, index);

			if (chr == *p_bdr)
			{
				p_bdr++;

				if (!*p_bdr)
				{
					return 1;
				}
			}
			else
			{
				p_bdr = boundary + (chr == boundary[0] ? 1 : 0);
			}
		}
	}
	return 0;
}
static char *MakeBoundary(void)
{
	char *boundary = strx(CRLF "--");
	uint count;

	for (count = 16; count; count--)
	{
		boundary = addLine_x(boundary, xcout("%02x", getCryptoByte()));
	}
	return boundary;
}
autoBlock_t *makeMultiPartContent(void)
{
	static char *boundary;
	uint index;
	autoBlock_t *content;

	errorCase(!TextList);

	while (!boundary || IsIncludeBoundary(boundary))
	{
		memFree(boundary);
		boundary = MakeBoundary();
	}
	content = newBlock();
	ab_addLine(content, boundary + 2);

	for (index = 0; index < getCount(TextList); index++)
	{
		ab_addLine(content, "\r\nContent-Disposition: form-data; name=\"");
		ab_addLine(content, getLine(TextList, index));
		ab_addLine(content, "\"");

		if (*getLine(FileNameList, index))
		{
			ab_addLine(content, " filename=\"");
			ab_addLine(content, getLine(FileNameList, index));
			ab_addLine(content, "\"");
		}
		ab_addLine(content, CRLF CRLF);
		ab_addBytes(content, (autoBlock_t *)getElement(BodyList, index));
		ab_addLine(content, boundary);
	}
	ab_addLine(content, "--" CRLF);

	releaseDim(TextList, 1);
	releaseDim(FileNameList, 1);
	releaseDim(BodyList, 1);
	TextList = NULL;

	return content;
}
