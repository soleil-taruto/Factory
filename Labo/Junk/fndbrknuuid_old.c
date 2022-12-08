#include "C:\Factory\Common\all.h"

#define UUID_FMT "{99999999-9999-9999-9999-999999999999}"

static uint GetMeaningCharPct(char *p)
{
	uint numer = 0;
	uint denom = 0;

	p++;

	while (*p != '}')
	{
		if (m_ishexadecimal(*p) || *p == '-')
			numer++;

		denom++;
		p++;
	}
	if (!denom)
		return 0;

	return (numer * 100) / denom;
}
static void FormatFltr(char *line)
{
	char *p;

	disctrl(line);

	for (p = line; *p; p++)
		if (m_ishexadecimal(*p))
			*p = '9';
}
static void FindBrokenUUID_File(char *file)
{
	autoList_t *lines = readLines(file);
	char *line;
	uint index;

	foreach (lines, line, index)
	{
		char *p = line;

		for (; ; )
		{
			p = strchr(p, '{');

			if (!p)
				break;

			// ? uuidらしきパターンを発見した。
			if (
				(
					lineExp("{<1,1,!\xff><>}<>", p) ||
					lineExp("{<><1,1,!\xff>}<>", p)
				)
				&& lineExp("{<10,,>}<>", p) && 70 <= GetMeaningCharPct(p)
				)
			{
				char *fmt = strx(p);

				FormatFltr(fmt);

				if (!startsWith(fmt, UUID_FMT)) // ? uuidと一致しない。
				{
					cout("%s %u\n", file, index + 1);
				}
				memFree(fmt);
			}
			p++;
		}
	}
	releaseDim(lines, 1);
}
static int IsTargetFile(char *file)
{
	char *ext;

	if (!_stricmp(file, getSelfFile()))
	{
//		LOGPOS();
		return 0;
	}
	ext = getExt(file);

	return
		!_stricmp(ext, "c") ||
		!_stricmp(ext, "h") ||
		!_stricmp(ext, "cs") ||
		!_stricmp(ext, "cpp") ||
		!_stricmp(ext, "java");
}
static void FindBrokenUUID(char *path)
{
	if (existDir(path))
	{
		autoList_t *files = lssFiles(path);
		char *file;
		uint index;

		foreach (files, file, index)
			if (IsTargetFile(file))
				FindBrokenUUID_File(file);

		releaseDim(files, 1);
	}
	else
	{
		FindBrokenUUID_File(path);
	}
}
int main(int argc, char **argv)
{
	antiSubversion = 1;

	FindBrokenUUID(hasArgs(1) ? nextArg() : c_dropDirFile());
}
