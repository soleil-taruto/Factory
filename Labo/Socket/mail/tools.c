#include "tools.h"

static char *GetFqdn(char *mailAddr)
{
	char *p;

	if (mailAddr)
	{
		p = mbs_strchr(mailAddr, '@');

		if (p)
			p++;
		else
			p = mailAddr;
	}
	else
	{
		p = "";
	}

	if (!*p)
		p = "default";

	return strx(p);
}
char *MakeMailMessageID(char *senderMailAddr)
{
	char *randPart = makeHexLine_x(makeCryptoRandBlock(16)); // 128bit
	char *fqdn = GetFqdn(senderMailAddr);
	char *messageID;

	messageID = xcout("<%I64u.%s@%s>", time(NULL), randPart, fqdn);
	memFree(randPart);
	memFree(fqdn);
	return messageID;
}
char *GetMailHeader(autoList_t *mail, char *key)
{
	char *value = NULL;
	char *line;
	uint index;

	foreach (mail, line, index)
	{
		char *p = strchr(line, ':');

		if (p)
		{
			p[0] = '\0';

			if (!_stricmp(line, key)) // ? found
			{
				char *q = p + 1;

				while (*q == ' ')
					q++;

				value = strx(q);
			}
			p[0] = ':';

			if (value)
			{
				while (++index < getCount(mail))
				{
					line = getLine(mail, index);

					if (line[0] != '\t' && line[0] != ' ') // ? not LWSP-char
						break;

					while (line[0] == '\t' || line[0] == ' ') // ? LWSP-char
						line++;

					value = addChar(value, ' ');
					value = addLine(value, line);
				}
				break;
			}
		}
	}
	return value;
}
char *RefMailHeader(autoList_t *mail, char *key)
{
	char *value = GetMailHeader(mail, key);

	if (!value)
		value = makeHexLine_x(makeCryptoRandBlock(32)); // dummy

	return value;
}
autoList_t *GetMailBody(autoList_t *mail)
{
	autoList_t *lines = newList();
	char *line;
	uint index;
	int enterBody = 0;

	foreach (mail, line, index)
	{
		if (enterBody)
		{
			addElement(lines, (uint)strx(line));
		}
		else if (line[0] == '\0')
		{
			enterBody = 1;
		}
	}
	return lines;
}

char *c_MakeMailMessageID(char *senderMailAddr)
{
	static char *stock;
	memFree(stock);
	return stock = MakeMailMessageID(senderMailAddr);
}
char *c_RefMailHeader(autoList_t *mail, char *key)
{
	static char *stock;
	memFree(stock);
	return stock = RefMailHeader(mail, key);
}
