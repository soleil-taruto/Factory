/*
	> sd

		�J�����g�f�B���N�g����K���Ȗ��O�i�ԍ��j�ŕۑ�����B

	> sd NAME

		�J�����g�f�B���N�g���� NAME �ŕۑ�����B

	> sd NAME DIR

		DIR �� NAME �ŕۑ�����B

	----

	�ۑ������f�B���N�g���ւ� ld NAME �ňړ�����B
*/

#include "C:\Factory\Common\all.h"
#include "libs\SaveDir.h"

static char *GetAutoName(void)
{
	char *sno = strx("0");

	createFileIfNotExist(SAVE_FILE);

	{
		autoList_t *lines = readLines(SAVE_FILE);
		char *line;
		uint index;

	research:
		for(index = 0; index < getCount(lines); index += 2)
			if(!_stricmp(sno, getLine(lines, index)))
				break;

		if(index < getCount(lines))
		{
			sno = xcout("%u", toValue_x(sno) + 1);
			goto research;
		}
	}

	return sno;
}
static char *GetAutoDir(void)
{
	return getCwd();
}
int main(int argc, char **argv)
{
	char *name = NULL;
	char *dir  = NULL;

	name = nnNextArg();
	dir  = nnNextArg();

	if(!name)
		name = GetAutoName(); // g

	if(!dir)
		dir = GetAutoDir(); // g

	line2JToken(name, 1, 1);
	errorCase(m_isEmpty(name));

	dir = makeFullPath(dir);
//	errorCase(!existDir(dir));

	mkAppDataDir();
	createFileIfNotExist(SAVE_FILE);

	{
		autoList_t *lines = readLines(SAVE_FILE);
		char *line;
		uint index;

		for(index = 0; index < getCount(lines); index += 2)
			if(!_stricmp(name, getLine(lines, index)))
				break;

		if(index < getCount(lines))
		{
			strzp((char **)directGetPoint(lines, index + 0), name);
			strzp((char **)directGetPoint(lines, index + 1), dir);
		}
		else
		{
			addElement(lines, (uint)strx(name));
			addElement(lines, (uint)strx(dir));
		}
		writeLines_cx(SAVE_FILE, lines);
	}

	memFree(dir);
}
