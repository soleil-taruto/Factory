#include "GmailSend.h"

static char *GetGmailSendExeFile(void)
{
	static char *file;

	if (!file)
//		file = GetCollaboFile("C:\\app\\Kit\\GmailSend\\GmailSend.exe"); // 廃止 @ 2023.4.13
		file = GetCollaboFile("C:\\Factory\\tmp\\Kit_Dummy\\GmailSend\\GmailSend.exe");

	return file;
}

// ---- Params ---

static autoList_t *ToList;
static autoList_t *CCList;
static autoList_t *BCCList;
static autoList_t *Attachments;
static char *From;
static char *Subject;
static char *Body;
static char *User;
static char *Password;
static char *Host;
static uint Port;
static int SSLDisabled;

static void INIT(void)
{
	{
		static int passed;

		if (passed)
			return;

		passed = 1;
	}

	ToList = newList();
	CCList = newList();
	BCCList = newList();
	Attachments = newList();
}
void GS_Clear(void)
{
	INIT();

	releaseDim(ToList, 1);
	releaseDim(CCList, 1);
	releaseDim(BCCList, 1);
	releaseDim(Attachments, 1);
	memFree(From);
	memFree(Subject);
	memFree(Body);
	memFree(User);
	memFree(Password);
	memFree(Host);

	ToList = newList();
	CCList = newList();
	BCCList = newList();
	Attachments = newList();
	From = NULL;
	Subject = NULL;
	Body = NULL;
	User = NULL;
	Password = NULL;
	Host = NULL;
	Port = 0;
	SSLDisabled = 0;
}
static char *FltrTokenParam(char *prm)
{
	errorCase(m_isEmpty(prm));

	INIT();

	prm = strx(prm);
	line2JToken(prm, 1, 1);
	prm = setStrLenMin(prm, 1, 'X');
	return prm;
}
void GS_AddTo(char *addr)
{
	addr = FltrTokenParam(addr);
	addElement(ToList, (uint)addr);
}
void GS_AddCC(char *addr)
{
	addr = FltrTokenParam(addr);
	addElement(CCList, (uint)addr);
}
void GS_AddBCC(char *addr)
{
	addr = FltrTokenParam(addr);
	addElement(BCCList, (uint)addr);
}
void GS_AddAttachment(char *file)
{
	errorCase(m_isEmpty(file));
	errorCase(!existFile(file));

	INIT();

	file = makeFullPath(file);
	addElement(Attachments, (uint)file);
}
void GS_SetFrom(char *addr)
{
	addr = FltrTokenParam(addr);
	strzp_x(&From, addr);
}
void GS_SetSubject(char *line)
{
	line = FltrTokenParam(line);
	strzp_x(&Subject, line);
}
void GS_SetBody(char *text)
{
	errorCase(!text);

	INIT();

	text = lineToJDoc(text, 1);
	strzp_x(&Body, text);
}
void GS_SetBody_x(char *text)
{
	GS_SetBody(text);
	memFree(text);
}
void GS_SetUser(char *line)
{
	line = FltrTokenParam(line);
	strzp_x(&User, line);
}
void GS_SetPassword(char *line)
{
	line = FltrTokenParam(line);
	strzp_x(&Password, line);
}
void GS_SetHost(char *line)
{
	line = FltrTokenParam(line);
	strzp_x(&Host, line);
}
void GS_SetPort(uint valPort)
{
	errorCase(!m_isRange(valPort, 0, 0xffff));
	Port = valPort;
}
void GS_SetSSLDisabled(int flag)
{
	SSLDisabled = flag;
}

// ----

int GS_TrySend(void) // ret: ? 成功
{
	char *cmdLine = xcout("START \"\" /B /WAIT \"%s\"", GetGmailSendExeFile());
	char *line;
	uint index;
	char *bodyFile = NULL;
	char *successfulFile = makeTempPath("gsSF.tmp");
	char *errorLogFile = makeTempPath("gsELF.tmp");
	int retval;
	char *prmFile;
	FILE *prmFp;

	INIT();

#if 1
	prmFile = makeTempPath("gsPrm.tmp");
	prmFp = fileOpen(prmFile, "wt");

	foreach (ToList, line, index)
	{
		writeLine(prmFp, "/To");
		writeLine(prmFp, line);
	}
	foreach (CCList, line, index)
	{
		writeLine(prmFp, "/CC");
		writeLine(prmFp, line);
	}
	foreach (BCCList, line, index)
	{
		writeLine(prmFp, "/BCC");
		writeLine(prmFp, line);
	}
	foreach (Attachments, line, index)
	{
		writeLine(prmFp, "/A");
		writeLine(prmFp, line);
	}
	if (From)
	{
		writeLine(prmFp, "/F");
		writeLine(prmFp, From);
	}
	if (Subject)
	{
		writeLine(prmFp, "/S");
		writeLine(prmFp, Subject);
	}
	if (Body)
	{
		bodyFile = makeTempPath("gsBody.tmp");
		writeOneLineNoRet_b(bodyFile, Body);

		writeLine(prmFp, "/B");
		writeLine(prmFp, bodyFile);
	}
	if (User && Password)
	{
		writeLine(prmFp, "/C");
		writeLine(prmFp, User);
		writeLine(prmFp, Password);
	}
	if (Host)
	{
		writeLine(prmFp, "/Host");
		writeLine(prmFp, Host);
	}
	if (Port)
	{
		writeLine(prmFp, "/Port");
		writeLine_x(prmFp, xcout("%u", Port));
	}
	if (SSLDisabled)
		writeLine(prmFp, "/-SSL");

	writeLine(prmFp, "/SF");
	writeLine(prmFp, successfulFile);
	writeLine(prmFp, "/ELF");
	writeLine(prmFp, errorLogFile);

	fileClose(prmFp);

	cmdLine = addLine_x(cmdLine, xcout(" //R \"%s\"", prmFile));

	coExecute(cmdLine);

	removeFile(prmFile);
	memFree(prmFile);
#else // コマンド引数に指定する版 -- 問題有り -> コマンドラインの長さ青天井。(")のエスケープをしていない。
	foreach (ToList, line, index)
		cmdLine = addLine_x(cmdLine, xcout(" /To \"%s\"", line));

	foreach (CCList, line, index)
		cmdLine = addLine_x(cmdLine, xcout(" /CC \"%s\"", line));

	foreach (BCCList, line, index)
		cmdLine = addLine_x(cmdLine, xcout(" /BCC \"%s\"", line));

	foreach (Attachments, line, index)
		cmdLine = addLine_x(cmdLine, xcout(" /A \"%s\"", line));

	if (From)
		cmdLine = addLine_x(cmdLine, xcout(" /F \"%s\"", From));

	if (Subject)
		cmdLine = addLine_x(cmdLine, xcout(" /S \"%s\"", Subject));

	if (Body)
	{
		bodyFile = makeTempPath("gsBody.tmp");
		writeOneLineNoRet_b(bodyFile, Body);
		cmdLine = addLine_x(cmdLine, xcout(" /B \"*%s\"", bodyFile));
	}
	if (User && Password)
		cmdLine = addLine_x(cmdLine, xcout(" /C \"%s\" \"%s\"", User, Password));

	if (Host)
		cmdLine = addLine_x(cmdLine, xcout(" /Host \"%s\"", Host));

	if (Port)
		cmdLine = addLine_x(cmdLine, xcout(" /Port %u", Port));

	if (SSLDisabled)
		cmdLine = addLine(cmdLine, " /-SSL");

	cmdLine = addLine_x(cmdLine, xcout(" /SF \"%s\"", successfulFile));
	cmdLine = addLine_x(cmdLine, xcout(" /ELF \"%s\"", errorLogFile));

	coExecute(cmdLine);
#endif

	if (existFile(successfulFile))
	{
		retval = 1;
		removeFile(successfulFile);
	}
	else
	{
		retval = 0;
	}

	if (existFile(errorLogFile))
	{
		char *errorLog = readText_b(errorLogFile);

		line2JLine(errorLog, 1, 1, 1, 1);

		cout("ERROR_LOG=[%s]\n", errorLog);

		memFree(errorLog);
		removeFile(errorLogFile);
	}
	if (bodyFile)
	{
		removeFile(bodyFile);
		memFree(bodyFile);
	}
	memFree(cmdLine);
	memFree(successfulFile);
	memFree(errorLogFile);

	cout("retval: %d\n", retval);

	/*
		連続してアクセスするとブロックされるんじゃないか？
		-> 成功・不成功に関係なくある程度待つ。
	*/
	coSleep(3000);

	return retval;
}
int GS_Send(void) // ret: ? 成功
{
	uint retry;

	for (retry = 0; retry < 3; retry++)
	{
		cout("GS_Send_retry: %d\n", retry);

		if (GS_TrySend())
		{
			cout("+----------+\n");
			cout("| 送信成功 |\n");
			cout("+----------+\n");
			return 1;
		}
	}
	cout("+------------+\n");
	cout("| 送信エラー |\n");
	cout("+------------+\n");
	return 0;
}
