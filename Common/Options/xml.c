/*
	読み込み失敗 -> error();
	エンコーディングは Shift_JIS, UTF-8, (US-ASCII) のみ対応する。
*/

#include "xml.h"

char *xmlAttrNameSuffix = ""; // Prefix だと NodeFltr で除去されてしまうので、、

// ---- Tools ----

static int IsSpaceOnly(char *text)
{
	char *p;

	for (p = text; *p; p++)
		if (!m_isspace(*p))
			return 0;

	return 1;
}
static void TrimSpace(char *text)
{
	char *p = strchr(text, '\0');

	for (; text < p; p--)
		if (!m_isspace(p[-1]))
			break;

	*p = '\0';

	for (p = text; *p; p++)
		if (!m_isspace(*p))
			break;

	copyLine(text, p);
}

// ---- In ----

static char *RData;
static uint RIndex;

// ---- Out ----

static int Encoding_UTF8;
static int NoDeclaration;
static XNode_t *CurrNode;
static autoList_t *NodeStack;

// ---- Funcs_Vars ----

static int RChr;
static char *RLine;
static autoList_t *RAttrNames;
static autoList_t *RAttrValues;

// ---- Funcs ----

static XNode_t *CreateNode(char *name, char *text, int mkcflg)
{
	XNode_t *node = nb_(XNode_t);

	if (name)
		node->Name = strx(name);

	if (text)
		node->Text = strx(text);

	if (mkcflg)
		node->Children = newList();

	return node;
}
static void ReadFileData(char *file, uint lenmax)
{
	FILE *fp = fileOpen(file, "rb");
	autoBlock_t *buff = newBlock();

	while (getSize(buff) < lenmax)
	{
		int chr = readChar(fp);

		if (chr == EOF)
			break;

		addByte(buff, chr);
	}
	fileClose(fp);
	RData = unbindBlock2Line(buff);
	RIndex = 0;
}
static int ReadChar(void)
{
	errorCase_m(RData[RIndex] == '\0', "XML:ファイルが途中で終わっています。");
	RChr = RData[RIndex++];
	return RChr;
}
static void BackChar(void)
{
	errorCase(!RIndex);
	RIndex--;
}
static int NextIs(char *nextPtn)
{
	return startsWithICase(RData + RIndex, nextPtn);
}
static void SkipTo(char *endChrs)
{
	while (!strchr(endChrs, ReadChar()));
}
static void SkipSpace(void)
{
	while (m_isspace(ReadChar()));
}
static void SkipToPtn(char *endPtn)
{
	while (!NextIs(endPtn))
		ReadChar();
}
static void Reset_RLine(void)
{
	if (!RLine)
		RLine = strx("");
	else
		*RLine = '\0';
}
static char *ReadLine(uint count)
{
	Reset_RLine();

	for (; count; count--)
		RLine = addChar(RLine, ReadChar());

	return RLine;
}
static char *ReadTo(char *endChrs, int spcEnd)
{
	Reset_RLine();

	for (; ; )
	{
		ReadChar();

		if (strchr(endChrs, RChr) || spcEnd && m_isspace(RChr))
			break;

		RLine = addChar(RLine, RChr);
	}
	return RLine;
}
/*
	RIndex_Pos:

	"aaaaa\"aaaaa"
	 ^            ^
	bgn          end
*/
static char *ReadLiteral(int quot)
{
	Reset_RLine();

	for (; ; )
	{
		ReadChar();

		if (_ismbblead(RChr))
		{
			RLine = addChar(RLine, RChr);
			ReadChar();
		}
		else if (RChr == '\\')
		{
			ReadChar();
		}
		else if (RChr == quot)
		{
			break;
		}
		RLine = addChar(RLine, RChr);
	}
	return RLine;
}
/*
	RIndex_Pos:

	<aaa bbb="ccc" ddd="eee">
	    ^                    ^
	   bgn                  end
*/
static void ReadAttributes(char *endChrs)
{
	if (RAttrNames)  releaseDim(RAttrNames, 1);
	if (RAttrValues) releaseDim(RAttrValues, 1);

	RAttrNames  = newList();
	RAttrValues = newList();

	for (; ; )
	{
		SkipSpace();

		if (strchr(endChrs, RChr))
			break;

		BackChar();
		ReadTo("=", 1);

		addElement(RAttrNames, (uint)RLine);
		RLine = NULL;

		BackChar();
		SkipTo("=");
		SkipSpace();

		if (RChr == '\'' || RChr == '"')
		{
			ReadLiteral(RChr);
		}
		else
		{
			BackChar();
			ReadTo(endChrs, 1);
			BackChar();
		}

		addElement(RAttrValues, (uint)RLine);
		RLine = NULL;
	}
}
static char *GetAttr(char *name)
{
	char *rAName;
	uint index;

	Reset_RLine();

	foreach (RAttrNames, rAName, index)
		if (!_stricmp(rAName, name))
			break;

	if (rAName)
		RLine = addLine(RLine, getLine(RAttrValues, index));

	return RLine;
}

// ---- Entity ----

static char *DecEntity_x(char *text)
{
	text = replaceLine(text, "&lt;", "<", 1);
	text = replaceLine(text, "&gt;", ">", 1);
	text = replaceLine(text, "&quot;", "\"", 1);
	text = replaceLine(text, "&apos;", "'", 1);
	text = replaceLine(text, "&amp;", "&", 1); // 最後に！

	return text;
}
static char *EncEntity(char *text)
{
	autoBlock_t *buff = newBlock();
	char *p;

	for (p = text; *p; p++)
	{
		switch (*p)
		{
		case '<': ab_addLine(buff, "&lt;"); break;
		case '>': ab_addLine(buff, "&gt;"); break;
		case '&': ab_addLine(buff, "&amp;"); break;
		case '"': ab_addLine(buff, "&quot;"); break;
		case '\'': ab_addLine(buff, "&apos;"); break;

		default:
			addByte(buff, *p);
			break;
		}
	}
	return unbindBlock2Line(buff);
}
static char *c_EncEntity(char *text)
{
	static char *stock;
	memFree(stock);
	return stock = EncEntity(text);
}

// ---- Read_Main ----

static void ReadXMLDeclaration(char *file)
{
	Encoding_UTF8 = 0;
	NoDeclaration = 0;

	ReadFileData(file, 65000);

	if (RData[0] == 0xEF) // ? UTF-8 の BOM
		ReadLine(3);

	ReadLine(5);

	if (!_stricmp(RLine, "<?xml"))
	{
		ReadAttributes("?");
		GetAttr("encoding");

		if (
			!_stricmp(RLine, "utf-8") ||
			!_stricmp(RLine, "utf_8") ||
			!_stricmp(RLine, "utf8")
			)
			Encoding_UTF8 = 1;
	}
	else
	{
		NoDeclaration = 1;
	}
	memFree(RData);
}
static void ReadXML(char *file)
{
	ReadFileData(file, UINTMAX);
	CurrNode = CreateNode(NULL, "", 1);
	NodeStack = newList();

	if (!NoDeclaration)
	{
		ReadLine(5);
		errorCase(_stricmp(RLine, "<?xml"));
		ReadAttributes("?");
		SkipSpace();

		errorCase_m(RChr != '>', "XML:宣言が閉じていないように見えます。");
	}

	for (; ; )
	{
		int tagClosed = 0;

		ReadTo("<", 0);

		CurrNode->Text = addLine(CurrNode->Text, RLine);

		SkipSpace();
		BackChar();

		if (NextIs("!DOCTYPE"))
		{
			RIndex += 8;
			ReadAttributes(">");
			continue;
		}
		if (NextIs("!--"))
		{
			RIndex += 3;
			SkipToPtn("-->");
			continue;
		}
		if (!NextIs("/")) // ? ! 閉じタグ
		{
			ReadTo("/>", 1);
			BackChar();

			// 子ノードに入る。
			{
				XNode_t *node = CreateNode(RLine, "", 1);
				addElement(NodeStack, (uint)CurrNode);
				addElement(CurrNode->Children, (uint)node);
				CurrNode = node;
			}

			ReadAttributes("/>");

			if (RChr == '/')
				tagClosed = 1;

			// 属性を子ノードとして追加
			{
				uint index;

				for (index = 0; index < getCount(RAttrNames); index++)
				{
					char *name  = getLine(RAttrNames,  index);
					char *value = getLine(RAttrValues, index);

					name = xcout("%s%s", name, xmlAttrNameSuffix);
					addElement(CurrNode->Children, (uint)CreateNode(name, value, 0));
					memFree(name);
				}
			}
		}
		else
		{
			tagClosed = 1;
		}

		if (tagClosed)
		{
			ReadTo(">", 1);
			CurrNode = (XNode_t *)unaddElement(NodeStack);

			if (getCount(NodeStack) == 0)
				break;
		}
	}

	errorCase_m(getCount(NodeStack), "XML:閉じていないタグがあります。"); // これは出ない。
	errorCase_m(getCount(CurrNode->Children) == 0, "XML:ルートタグがありません。");

	// CurrNode: ダミールート -> 本当のルート
	{
		XNode_t *root = (XNode_t *)unaddElement(CurrNode->Children); // リリースするので取り除く
		ReleaseXNode(CurrNode);
		CurrNode = root;
	}

	memFree(RData);
	// CurrNode <- ret
	releaseAutoList(NodeStack);
}

static void NodeFltr(XNode_t *node)
{
	char *p = strrchr(node->Name, ':');

	if (p)
		copyLine(node->Name, p + 1); // ネームスペースを除去

	errorCase_m(!*node->Name, "XML:名前の無いタグ又は属性があります。");

	node->Text = DecEntity_x(node->Text);
}
static void AllNodeFltr(XNode_t *node)
{
	autoList_t *nodeStack = newList();

	addElement(nodeStack, (uint)node);

	while (getCount(nodeStack))
	{
		node = (XNode_t *)unaddElement(nodeStack);
		NodeFltr(node);
		addElements(nodeStack, node->Children);
	}
	releaseAutoList(nodeStack);
}

// ---- Write_Main ----

static FILE *Fp;

static void WrIndent(uint count)
{
	for (; count; count--)
		writeChar(Fp, '\t');
}
static void WriteNode(XNode_t *node, uint indent)
{
	if (getCount(node->Children))
	{
		WrIndent(indent);
		writeLine_x(Fp, xcout("<%s>", node->Name));

		// 子要素を出力
		{
			XNode_t *child;
			uint index;

			foreach (node->Children, child, index)
				WriteNode(child, indent + 1);
		}

		if (!getCount(node->Children) || !IsSpaceOnly(node->Text))
		{
			char *text = EncEntity(node->Text);

			TrimSpace(text);

			WrIndent(indent + 1);
			writeLine(Fp, text);

			memFree(text);
		}
		WrIndent(indent);
		writeLine_x(Fp, xcout("</%s>", node->Name));
	}
	else
	{
		WrIndent(indent);

		if (*node->Text)
			writeLine_x(Fp, xcout("<%s>%s</%s>", node->Name, c_EncEntity(node->Text), node->Name));
		else
			writeLine_x(Fp, xcout("<%s/>", node->Name));
	}
}

// ----

XNode_t *readXMLFile(char *file)
{
	XNode_t *root;

	ReadXMLDeclaration(file);
cout("Encoding_UTF8: %d\n", Encoding_UTF8); // test

	if (Encoding_UTF8)
	{
		char *tmpFile = makeTempPath("xml");

		UTF8ToSJISFile(file, tmpFile);

		ReadXML(tmpFile);
		removeFile(tmpFile);
		memFree(tmpFile);
	}
	else
	{
		ReadXML(file);
	}
	root = CurrNode;
	NormalizeXNode(root, NULL);
	AllNodeFltr(root);
	return root;
}
void writeXMLFile(char *file, XNode_t *root)
{
	Fp = fileOpen(file, "wt");
	writeLine_x(Fp, xcout("<?xml version=\"1.0\" encoding=\"Shift_JIS\"?>"));
	WriteNode(root, 0);
	fileClose(Fp);
}

// _x
void writeXMLFile_cx(char *file, XNode_t *root)
{
	writeXMLFile(file, root);
	ReleaseXNode(root);
}
