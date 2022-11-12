#include "GitSourceFilter.h"

#define COMMAND_PREFIX "$""_git:" // ‚±‚±‚ğŒŸo‚µ‚È‚¢‚æ‚¤‚ÉAŠÔ‚É ""
#define COMMAND_SECRET "secret"
#define COMMAND_SECRET_BEGIN "secretBegin"
#define COMMAND_SECRET_END "secretEnd"

static char *MaskLine(char *line, char *end)
{
	char *p = line;
	uint span;

	while (p < end && *p <= ' ')
		p++;

	while (p < end && end[-1] <= ' ')
		end--;

	span = (uint)end - (uint)p;

	while (p < end)
		*p++ = '/';

	if (span < 2)
		line = insertLine(line, (uint)end - (uint)line, "//" + span);

	return line;
}
static FilterSourceFile(char *file)
{
	autoList_t *lines = readLines(file);
	char *line;
	uint index;
	int onSecret = 0;

	cout("* %s\n", file);

	foreach (lines, line, index)
	{
		char *p = strstr(line, COMMAND_PREFIX);
		int onSecretLater = 0;

		if (p)
		{
			char *command = strx(p + strlen(COMMAND_PREFIX));
			char *q;

			ucTrim(command);
			q = strchr(command, ' ');

			if (q)
				*q = '\0';

			if (!strcmp(command, COMMAND_SECRET))
			{
				setElement(lines, index, (uint)MaskLine(line, p));
			}
			else if (!strcmp(command, COMMAND_SECRET_BEGIN))
			{
				onSecretLater = 1;
			}
			else if (!strcmp(command, COMMAND_SECRET_END))
			{
				onSecret = 0;
			}
			// ‚±‚±‚Ö’Ç‰Á...
			else
				error();

			memFree(command);
		}
		if (onSecret)
		{
			setElement(lines, index, (uint)MaskLine(line, strchr(line, '\0')));
		}
		onSecret |= onSecretLater;
	}
	writeLines_cx(file, lines);

	errorCase_m(onSecret, "secret is not closed !");
}
void GitSourceFilter(char *rootDir)
{
	autoList_t *files = lssFiles(rootDir);
	char *file;
	uint index;

	LOGPOS();

	RemoveGitPaths(files);

	foreach (files, file, index)
	{
		char *ext = getExt(file);

		if (
			!_stricmp(ext, "bat") ||
			!_stricmp(ext, "c") ||
			!_stricmp(ext, "h") ||
			!_stricmp(ext, "cs") ||
			!_stricmp(ext, "cpp") ||
			!_stricmp(ext, "java")
			)
			FilterSourceFile(file);
	}
	releaseDim(files, 1);

	LOGPOS();
}
