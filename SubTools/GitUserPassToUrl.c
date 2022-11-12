#include "C:\Factory\Common\all.h"

static char *RootDir = "C:\\huge\\GitHub";
static uint SearchDeep = 1;
static char *User = "user01";
static char *Pass = "9999";
static char *InsertPtn;

#define TARGET_KEY "url"
#define TARGET_VALUE_PREFIX "https://"
#define TARGET_VALUE_SUFFIX_A "@github.com/"

static void FoundCfgFile(char *cfgFile)
{
	autoList_t *lines = readLines(cfgFile);
	char *line;
	uint index;
	int mod = 0;

	cout("C %s\n", cfgFile);

	foreach (lines, line, index)
	{
		char *tmpLine = strx(line);
		char *key;
		char *value;

		key = toknext(tmpLine, "=");
		value = toknext(NULL, "");

		if (value)
		{
			char *tKey = strx(key);

			ucTrim(tKey);

			if (!strcmp(tKey, TARGET_KEY))
			{
				char *p = strstrNext(value, TARGET_VALUE_PREFIX);

				if (p)
				{
					char *q = strstr(p, TARGET_VALUE_SUFFIX_A);

					if (q)
						q++;
					else
						q = strstr(p, TARGET_VALUE_SUFFIX_A + 1);

					if (q)
					{
						if (p + strlen(InsertPtn) == q && !memcmp(p, InsertPtn, strlen(InsertPtn)))
						{
							cout("変更されていません。\n");
						}
						else
						{
							q = strx(q); // p == q ということもあるよ。
							*p = '\0';
							p = xcout("%s=%s%s%s", key, value, InsertPtn, q);

							cout("< %s\n", line);

							memFree(line);
							setElement(lines, index, (uint)p);
							mod = 1;

							cout("> %s\n", getElement(lines, index)); // show p

							memFree(q);
						}
					}
				}
			}
			memFree(tKey);
		}
		memFree(tmpLine);
	}
	cout("mod: %d\n", mod);

	if (mod)
		writeLines_b_cx(cfgFile, lines); // .git\config の改行は LF
	else
		releaseDim(lines, 1);
}
static void Search(char *currDir, uint remDeep)
{
	char *cfgFile = combine(currDir, ".git\\config");

	if (existFile(cfgFile))
	{
		FoundCfgFile(cfgFile);
	}
	else if (remDeep)
	{
		autoList_t *dirs = lsDirs(currDir);
		char *dir;
		uint index;

		foreach (dirs, dir, index)
			Search(dir, remDeep - 1);

		releaseDim(dirs, 1);
	}
}
int main(int argc, char **argv)
{
readArgs:
	if (argIs("/R"))
	{
		RootDir = nextArg();
		goto readArgs;
	}
	if (argIs("/D"))
	{
		SearchDeep = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/U"))
	{
		User = nextArg();
		goto readArgs;
	}
	if (argIs("/P"))
	{
		Pass = nextArg();
		goto readArgs;
	}
	if (argIs("/C"))
	{
		User = "";
		Pass = "";
		goto readArgs;
	}

	errorCase(!existDir(RootDir));
	errorCase(IMAX < SearchDeep);
	errorCase((*User || *Pass) && (!*User || !*Pass));

	if (*User)
		InsertPtn = xcout("%s:%s@", User, Pass);
	else
		InsertPtn = "";

	Search(RootDir, SearchDeep);
}
