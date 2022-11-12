#include "C:\Factory\Common\all.h"

static int Mode = 'B'; // "BGS" == Both, Getter only, Setter only

static void RemovePublicEtc(autoList_t *tokens)
{
	while (getCount(tokens))
	{
		char *token = getLine(tokens, 0);

		if (
			strcmp(token, "final") &&
			strcmp(token, "static") &&
			strcmp(token, "private") &&
			strcmp(token, "protected") &&
			strcmp(token, "public")
			)
			break;

		desertElement(tokens, 0);
		memFree(token);
	}
}
static char *GetVirName(char *name)
{
	char *ret = strx(name);

	if (*ret == '_')
		eraseChar(ret);

	*ret = m_toupper(*ret);
	return ret;
}
static char *GetPrmName(char *name)
{
	char *ret = strx(name);

	if (*ret == '_')
		eraseChar(ret);

	return ret;
}
int main(int argc, char **argv)
{
	autoList_t *lines;
	char *line;
	uint index;
	autoList_t *outLines = newList();

readArgs:
	if (argIs("/Go") || argIs("/-S")) // Getter only
	{
		Mode = 'G';
		goto readArgs;
	}
	if (argIs("/So") || argIs("/-G")) // Setter only
	{
		Mode = 'S';
		goto readArgs;
	}
	errorCase_m(hasArgs(1), "Unknown args");

	lines = inputLines();

	foreach (lines, line, index)
	{
		char *p = strchr(line, ';');

		if (p)
		{
			autoList_t *tokens;

			*p = '\0';
			p = strchr(line, '=');

			if (p)
				*p = '\0';

			tokens = ucTokenize(line);

			RemovePublicEtc(tokens);

			if (2 <= getCount(tokens))
			{
				char *name = (char *)desertElement(tokens, getCount(tokens) - 1);
				char *virName;
				char *prmName;
				char *type;
				char *setterLeftPrfx;

				virName = GetVirName(name);
				prmName = GetPrmName(name);
				type = untokenize(tokens, " ");

				if (!strcmp(name, prmName))
					setterLeftPrfx = "this.";
				else
					setterLeftPrfx = "";

				if (Mode != 'S') // ? ! Setter only -> output Getter
				{
					addElement(outLines, (uint)xcout("public %s get%s() {", type, virName));
					addElement(outLines, (uint)xcout("\treturn %s;", name));
					addElement(outLines, (uint)xcout("}"));
					addElement(outLines, (uint)xcout(""));
				}
				if (Mode != 'G') // ? ! Getter only -> output Setter
				{
					addElement(outLines, (uint)xcout("public void set%s(%s %s) {", virName, type, prmName));
					addElement(outLines, (uint)xcout("\t%s%s = %s;", setterLeftPrfx, name, prmName));
					addElement(outLines, (uint)xcout("}"));
					addElement(outLines, (uint)xcout(""));
				}
				memFree(name);
				memFree(virName);
				memFree(prmName);
				memFree(type);
			}
			releaseDim(tokens, 1);
		}
	}
	if (getCount(outLines))
	{
		viewLines(outLines);
	}
	releaseDim(lines, 1);
	releaseDim(outLines, 1);
}
