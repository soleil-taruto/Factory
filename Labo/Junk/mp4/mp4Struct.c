#include "C:\Factory\Common\all.h"

static autoBlock_t *FileData;

static void PrintIndent(uint indent)
{
	uint c;

	for (c = 0; c < indent; c++)
		cout("\t");
}
static void PrintSubStruct(uint startPos, uint endPos, uint indent)
{
	uint index = startPos;

	while (index < endPos)
	{
		uint size;
		char *name;

		if (endPos < index + 8)
		{
			PrintIndent(indent);
			cout("中止：残り領域が小さすぎる。\n");
			break;
		}
		size = ab_getValueBE(FileData, index);

		if (size < 8)
		{
			PrintIndent(indent);
			cout("中止：サイズが小さすぎる。\n");
			break;
		}
		if (endPos < index + size)
		{
			PrintIndent(indent);
			cout("中止：サイズが大きすぎる。\n");
			break;
		}
		name = (char *)memAlloc(5);
		memcpy(name, b_(FileData) + index + 4, 4);
		name[4] = '\0';

		line2JLine(name, 1, 0, 0, 1);

		PrintIndent(indent);
		cout("[%s] %u\n", name, size);

		PrintSubStruct(index + 8, index + size, indent + 1);

		memFree(name);

		index += size;
	}
}
static void PrintMp4Struct_File(char *file)
{
	cout("< %s\n", file);

	FileData = readBinary(file);

	PrintSubStruct(0, getSize(FileData), 0);

	releaseAutoBlock(FileData);
	FileData = NULL;

	cout("\n");
}
static void PrintMp4Struct(char *path)
{
	if (existDir(path))
	{
		autoList_t *files = lssFiles(path);
		char *file;
		uint index;

		foreach (files, file, index)
			PrintMp4Struct_File(file);

		releaseDim(files, 1);
	}
	else
	{
		PrintMp4Struct_File(path);
	}
}
int main(int argc, char **argv)
{
	if (hasArgs(1))
	{
		PrintMp4Struct(nextArg());
		return;
	}

	for (; ; )
	{
		char *path = c_dropDirFile();

		PrintMp4Struct(path);
		cout("\n");
	}
}
