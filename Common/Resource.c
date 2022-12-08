/*
	readResourceLines()
		�S�s ucTrimEdge() ������ŁA��s�� COMMENT_LEADER �Ŏn�܂�s�͎�菜���B

	readResourcePaths()
		readResourceLines() -> �S�s��΃p�X�ɕϊ�����B
		���΂̊�� file �̏ꏊ�Ƃ���B

	readResourceFilesLines()
		readResourcePaths() -> �S�s readResourceLines()
		�J������ɂ� releaseDim(ret, 2);

	readResourceLinesList()
		readResourceLines()
		-> SEPARATOR_LINE �ƈ�v����s���Z�p���[�^�Ƃ��āA�s���X�g�̃��X�g�ɂ���B
		��̍s���X�g�͎�菜���B
		�J������ɂ� releaseDim(ret, 2);
*/

#include "all.h"

#define COMMENT_LEADER ';'
#define SEPARATOR_LINE "/"

void disctrl(char *line)
{
	char *p;

	for (p = line; *p; )
	{
		if (*p < ' ')
			eraseChar(p);
		else
			p++;
	}
}
void unctrl(char *line)
{
	char *p;

	for (p = line; *p; p++)
	{
		if (*p < ' ')
		{
			*p = ' ';
		}
	}
}
void ucTrim(char *line)
{
	unctrl(line);
	trim(line, ' ');
}
void ucTrimEdge(char *line)
{
	unctrl(line);
	trimEdge(line, ' ');
}
void ucTrimLead(char *line)
{
	unctrl(line);
	trimLead(line, ' ');
}
void ucTrimTrail(char *line)
{
	unctrl(line);
	trimTrail(line, ' ');
}
void ucTrimSqLead(char *line)
{
	unctrl(line);
	trimSequ(line, ' ');
	trimLead(line, ' ');
}
void ucTrimSqTrail(char *line)
{
	unctrl(line);
	trimSequ(line, ' ');
	trimTrail(line, ' ');
}
autoList_t *ucTokenize(char *line)
{
	autoList_t *tokens;

	line = strx(line);
	unctrl(line);
	tokens = tokenize(line, ' ');
	memFree(line);
	trimLines(tokens);
	return tokens;
}
autoList_t *ucTokenizeDs(char *line, char *delims)
{
	autoList_t *tokens = tokenizeDs(line, delims);
	char *token;
	uint index;

	foreach (tokens, token, index)
		ucTrimEdge(token);

	trimLines(tokens);
	return tokens;
}
void removeBlank(char *line)
{
	unctrl(line);
	removeChar(line, ' ');
}

void ucTrimAllLine(autoList_t *lines)
{
	callAllElement(lines, (void (*)(uint))ucTrim);
}
void ucTrimEdgeAllLine(autoList_t *lines)
{
	callAllElement(lines, (void (*)(uint))ucTrimEdge);
}
void ucTrimSqLeadAllLine(autoList_t *lines)
{
	callAllElement(lines, (void (*)(uint))ucTrimSqLead);
}
void ucTrimSqTrailAllLine(autoList_t *lines)
{
	callAllElement(lines, (void (*)(uint))ucTrimSqTrail);
}

autoList_t *readResourceLines(char *file)
{
	autoList_t *lines = readLines(file);
	char *line;
	uint index;

	foreach (lines, line, index)
	{
		ucTrimEdge(line);

		if (line[0] == COMMENT_LEADER) // COMMENT_LEADER �Ŏn�܂�s���w�肵�����ꍇ�� '"' �Ŏn�߂�B
		{
			line[0] = '\0';
		}
	}
	trimLines(lines);

	foreach (lines, line, index) // �擪�ƏI�[�� '"' ����������B�󕶎��񂪎w��\�ɂȂ�B'"' �Ŏn�܂�E�I���s���w�肵�����ꍇ�� '"' ���Q�d�˂�B
	{
		char *p = strchr(line, '\0') - 1; // line == "" �͗L�蓾�Ȃ��B

		if (*p == '"')
			*p = '\0';

		if (line[0] == '"')
			eraseChar(line);
	}
	return lines;
}
autoList_t *readResourcePaths(char *pathListFile)
{
	autoList_t *paths;
	char *path;
	uint index;

	pathListFile = makeFullPath(pathListFile);
	paths = readResourceLines(pathListFile);

	addCwd_x(getParent(pathListFile));

	foreach (paths, path, index)
	{
		setElement(paths, index, (uint)makeFullPath(path));
		memFree(path);
	}
	unaddCwd();
	memFree(pathListFile);
	return paths;
}
autoList_t *readResourceFilesLines(char *fileListFile)
{
	autoList_t *files = readResourcePaths(fileListFile);
	char *file;
	uint index;
	autoList_t *linesList = newList();

	foreach (files, file, index)
	{
		addElement(linesList, (uint)readResourceLines(file));
	}
	releaseDim(files, 1);
	return linesList;
}
autoList_t *readResourceLinesList(char *file)
{
	autoList_t *lines = readResourceLines(file);
	autoList_t *linesList = newList();
	autoList_t *store = newList();
	char *line;
	uint index;

	foreach (lines, line, index)
	{
		if (!strcmp(line, SEPARATOR_LINE))
		{
			memFree(line);
			addElement(linesList, (uint)store);
			store = newList();
		}
		else
		{
			addElement(store, (uint)line);
		}
	}
	addElement(linesList, (uint)store);
	releaseAutoList(lines);

	foreach (linesList, lines, index)
	{
		if (getCount(lines) == 0)
		{
			releaseAutoList(lines);
			setElement(linesList, index, 0);
		}
	}
	removeZero(linesList);
	return linesList;
}
autoList_t *discerpHeaders(autoList_t *linesList)
{
	autoList_t *headers = newList();
	autoList_t *lines;
	uint index;

	foreach (linesList, lines, index)
	{
		addElement(headers, desertElement(lines, 0));
	}
	return headers;
}
char *readResourceText(char *file)
{
	return untokenize_xc(readResourceLines(file), "\n");
}
char *readResourceArgsText(char *file)
{
	return untokenize_xc(readResourceLines(file), " ");
}
char *readResourceOneLine(char *file)
{
	return untokenize_xc(readResourceLines(file), "");
}

// _x
autoList_t *readResourceLines_x(char *file)
{
	autoList_t *ret = readResourceLines(file);
	memFree(file);
	return ret;
}
