#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\UTF.h"

static char *WkFile_A;
static char *WkFile_B;

static autoBlock_t *Prv_UTF8ToUTF16Line(char *line)
{
	writeOneLineNoRet_b(WkFile_A, line);

	UTF8ToSJISFile(WkFile_A, WkFile_A);

	UTF_BE = 0;
	UTF_NoWriteBOM = 1;
	SJISToUTF16File(WkFile_A, WkFile_B);

	return readBinary(WkFile_B);
}
static char *Prv_UTF16ToUTF8Line(autoBlock_t *u16Line)
{
	writeBinary(WkFile_A, u16Line);

	UTF_BE = 0;
	UTF16ToSJISFile(WkFile_A, WkFile_B);

	SJISToUTF8File(WkFile_B, WkFile_B);

	return readText_b(WkFile_B);
}
static void U16Line_LTrim(autoBlock_t *u16Line)
{
	while (
		2 <= getSize(u16Line) &&
		b_(u16Line)[0] <= ' ' &&
		b_(u16Line)[1] == 0x00
		)
		removeBytes(u16Line, 0, 2);
}
static void U16Line_AddAsciiLine(autoBlock_t *u16Line, char *line)
{
	char *p;

	for (p = line; *p; p++)
	{
		addByte(u16Line, *p);
		addByte(u16Line, 0x00);
	}
}
static void JavaJPConvFile(char *file)
{
	autoList_t *lines = readLines(file);
	char *line;
	uint line_index;
	int modified = 0;

	cout("file: %s\n", file);

	WkFile_A = makeTempPath(NULL);
	WkFile_B = makeTempPath(NULL);

	foreach (lines, line, line_index)
	{
		autoBlock_t *u16Line = Prv_UTF8ToUTF16Line(line);
		uint index;
		int line_modified = 0;
		autoBlock_t *dest = newBlock();

		for (index = 0; index + 2 <= getSize(u16Line); index += 2)
		{
			if (
				index + 4 <= getSize(u16Line) &&
				b_(u16Line)[index + 0] == '\\' && b_(u16Line)[index + 1] == 0x00 &&
				b_(u16Line)[index + 2] == '\\' && b_(u16Line)[index + 3] == 0x00
				)
			{
				addByte(dest, '\\');
				addByte(dest, 0x00);
				addByte(dest, '\\');
				addByte(dest, 0x00);

				index += 2;
			}
			else if (
				index + 12 <= getSize(u16Line) &&
				b_(u16Line)[index + 0] == '\\' && b_(u16Line)[index + 1] == 0x00 &&
				b_(u16Line)[index + 2] == 'u'  && b_(u16Line)[index + 3] == 0x00 &&
				m_ishexadecimal(b_(u16Line)[index +  4]) && b_(u16Line)[index +  5] == 0x00 &&
				m_ishexadecimal(b_(u16Line)[index +  6]) && b_(u16Line)[index +  7] == 0x00 &&
				m_ishexadecimal(b_(u16Line)[index +  8]) && b_(u16Line)[index +  9] == 0x00 &&
				m_ishexadecimal(b_(u16Line)[index + 10]) && b_(u16Line)[index + 11] == 0x00
				)
			{
				uint u16ch =
					m_c2i(b_(u16Line)[index +  4]) << 12 |
					m_c2i(b_(u16Line)[index +  6]) <<  8 |
					m_c2i(b_(u16Line)[index +  8]) <<  4 |
					m_c2i(b_(u16Line)[index + 10]);

				addByte(dest, u16ch & 0xff);
				addByte(dest, u16ch >> 8);

				index += 10;
				line_modified = 1;
				modified = 1;
			}
			else
			{
				addByte(dest, b_(u16Line)[index + 0]);
				addByte(dest, b_(u16Line)[index + 1]);
			}
		}
		if (line_modified)
		{
			U16Line_LTrim(u16Line);
			U16Line_AddAsciiLine(dest, " // orig: ");
			addBytes(dest, u16Line);

			memFree(line);
			line = Prv_UTF16ToUTF8Line(dest);

			setElement(lines, line_index, (uint)line);
		}
	}
	removeFile_x(WkFile_A);
	removeFile_x(WkFile_B);
	WkFile_A = NULL;
	WkFile_B = NULL;

	if (modified)
	{
		cout("ファイルを更新します。\n");
		writeLines(file, lines);
	}
	releaseDim(lines, 1);
}
static int AcceptName(char *name)
{
//cout("%s\n", name); // test
	return name[0] != '.'; // .git など除外する。
}
static void JavaJPConv(char *rootDir)
{
	autoList_t *files;
	char *file;
	uint index;

	findAcceptName = AcceptName;
	files = lssFiles(rootDir);
	findAcceptName = NULL; // restore

	foreach (files, file, index)
		if (!_stricmp(getExt(file), "java"))
			JavaJPConvFile(file);

	releaseDim(files, 1);
}
int main(int argc, char **argv)
{
	JavaJPConv(nextArg());
}
