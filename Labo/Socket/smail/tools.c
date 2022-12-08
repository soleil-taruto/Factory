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

// ---- MailParser ----

static autoList_t *MP_HeaderKeys;
static autoList_t *MP_HeaderValues;
static autoBlock_t *MP_Body;

void MailParser(autoBlock_t *mail)
{
	uint rPos;

	errorCase(!mail);

	MP_Clear();

	MP_HeaderKeys = newList();
	MP_HeaderValues = newList();

	for (rPos = 0; ; )
	{
		uint i = rPos;
		char *headerLine;
		int folding;

		for (; i < getSize(mail); i++)
			if (b_(mail)[i] == '\n')
				break;

		if (i == getSize(mail))
		{
			cout("ヘッダの終端が見つかりません。\n");
			break;
		}
		b_(mail)[i] = '\0';
		headerLine = (char *)b_(mail) + rPos;
		rPos = i + 1;
		folding = m_isspace(headerLine[0]) && getCount(MP_HeaderKeys);
		ucTrimEdge(headerLine);

		if (!*headerLine)
			break;

		if (folding)
		{
			char *lastValue = (char *)unaddElement(MP_HeaderValues);

			lastValue = addChar(lastValue, ' ');
			lastValue = addLine(lastValue, headerLine);

			addElement(MP_HeaderValues, (uint)lastValue);
		}
		else
		{
			char *p = strchr(headerLine, ':');

			if (p)
			{
				char *key = headerLine;
				char *value;

				*p = '\0';
				value = p + 1;

				ucTrimEdge(key);
				ucTrimEdge(value);

				addElement(MP_HeaderKeys,   (uint)strx(key));
				addElement(MP_HeaderValues, (uint)strx(value));
			}
			else
			{
				line2JLine(headerLine, 1, 0, 0, 1); // 表示のため
				cout("不明なヘッダ行 = [%s]\n", headerLine);
			}
		}
	}
	MP_Body = getFollowBytes(mail, rPos);
}
void MP_Clear(void)
{
	if (!MP_HeaderKeys) // ? メール未展開
		return;

	// 2bs
	errorCase(!MP_HeaderKeys);
	errorCase(!MP_HeaderValues);
	errorCase(!MP_Body);

	releaseDim(MP_HeaderKeys, 1);
	releaseDim(MP_HeaderValues, 1);
	releaseAutoBlock(MP_Body);

	MP_HeaderKeys = NULL;
	MP_HeaderValues = NULL;
	MP_Body = NULL;
}
char *MP_GetHeaderValue(char *targKey) // ret: strx(), NULL == 見つからない。
{
	char *key;
	uint index;

	errorCase(m_isEmpty(targKey));

	// ? メール未展開
	errorCase(!MP_HeaderKeys);
	errorCase(!MP_HeaderValues); // 2bs

	foreach (MP_HeaderKeys, key, index)
		if (!_stricmp(key, targKey))
			return strx(getLine(MP_HeaderValues, index));

	return NULL;
}
autoBlock_t *c_MP_GetBody(void)
{
	errorCase(!MP_Body); // ? メール未展開

	return MP_Body;
}

// ----

void PrintMailEntity(autoBlock_t *mail)
{
	char *line;

	if (31000 < getSize(mail)) // ? 31 KB より大きい
	{
		autoBlock_t *buff = newBlock();

		ab_addSubBytes(buff, mail, 0, 20000);                 // 最初の 20 KB
		ab_addLine(buff, " ... ");
		ab_addFollowBytes(buff, mail, getSize(mail) - 10000); // 最後の 10 KB

		line = toPrintLine_x(buff, 1);
	}
	else
	{
		line = toPrintLine(mail, 1);
	}

	cout(
		"mail-size: %u bytes\n"
		"▼▼▼メールの内容ここから▼▼▼\n"
		"%s\n"
		"▲▲▲メールの内容ここまで▲▲▲\n"
		,getSize(mail)
		,line
		);

	memFree(line);
}
