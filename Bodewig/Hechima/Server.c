/*
	使い方

		Server.exe 52525
		crypTunnel.exe 52255 localhost 52525 /C 25 /R *aa9999x22x-PASSWORD_TRAILER

		クライアント側は、サーバーの 52255 ポートに、パスワード aa9999x22x で接続する。

		"-PASSWORD_TRAILER" の部分は、HechimaClient の Consts.cs の PASSWORD_TRAILER と同じであること。
*/

#include "C:\Factory\Common\Options\SockServerTh.h"
#include "C:\Factory\Common\Options\SockStream.h"

#define REMARKS_SAVE_FILE "C:\\Factory\\tmp\\HechimaServerRemarks.txt"
//#define REMARKS_SAVE_FILE "C:\\appdata\\HechimaServerRemarks.txt" // appdata 抑止 @ 2021.3.19

#define COMMAND_LENMAX 50
#define SVAL64_LENMAX 30
#define NAME_LENMAX 100
#define IDENT_LENMAX (NAME_LENMAX + 30)
#define MESSAGE_LENMAX 1024

#define REMARK_MAX 1000
#define REMARK_CLEAR_PCT 20
#define REMARK_CLEAR_NUM ((REMARK_MAX * REMARK_CLEAR_PCT) / 100)

#define MEMBER_MAX 25

typedef struct Remark_st
{
	uint64 Stamp;
	char *Ident;
	char *Message;
}
Remark_t;

typedef struct Member_st
{
	uint LastAccessTime;
	char *Ident;
}
Member_t;

static autoList_t *Remarks;
static autoList_t *Members;

static void ReleaseRemark(Remark_t *i)
{
	memFree(i->Ident);
	memFree(i->Message);
	memFree(i);
}
static void ReleaseMember(Member_t *i)
{
	memFree(i->Ident);
	memFree(i);
}
static void SaveRemarks(void)
{
	FILE *fp = fileOpen(REMARKS_SAVE_FILE, "wt");
	Remark_t *i;
	uint index;

	foreach (Remarks, i, index)
	{
		writeLine_x(fp, xcout("%I64u", i->Stamp));
		writeLine(fp, i->Ident);
		writeLine(fp, i->Message);
	}
	fileClose(fp);
}
static void LoadRemarks(void)
{
	if (existFile(REMARKS_SAVE_FILE))
	{
		FILE *fp = fileOpen(REMARKS_SAVE_FILE, "rt");

		for (; ; )
		{
			char *line = readLine(fp);
			Remark_t *i;

			if (!line)
				break;

			errorCase(REMARK_MAX + 10 < getCount(Remarks)); // + margin <

			i = nb_(Remark_t);
			i->Stamp = toValue64_x(line);
			i->Ident = nnReadLine(fp);
			i->Message = nnReadLine(fp);

			// 補正
			{
				m_range(i->Stamp, 10000101000000ui64, 99991231235959ui64);

				if (getCount(Remarks))
				{
					uint64 nxtLwStmp = ((Remark_t *)getLastElement(Remarks))->Stamp + 1ui64;

					m_maxim(i->Stamp, nxtLwStmp);
				}

				setStrLenMax(i->Ident, IDENT_LENMAX);
				line2JLine(i->Ident, 1, 0, 0, 1); // " @ " があるので、okSpc == 1

				setStrLenMax(i->Message, MESSAGE_LENMAX);
				line2JLine(i->Message, 1, 0, 0, 1);
				trimEdge(i->Message, ' ');
			}

			addElement(Remarks, (uint)i);
		}
		fileClose(fp);
	}
}
static char *GetNowStr(void)
{
	static char buff[12]; // maxlen: "71582788:15" <- now() == 0xffffffff のとき
	uint m;
	uint s = now();

	m = s / 60;
	s %= 60;

	sprintf(buff, "%u:%02u", m, s);

	return buff;
}
static char *GetPseudoIP(char *name) // なんちゃってIPを返す。
{
	static char buff[16]; // maxlen: "255.255.255.255" <--- 実際は "99.99.99.99" -- へちま改のためにIPは常に11文字でなければならない。

	if (4 <= strlen(name))
	{
		uchar *p = strchr(name, '\0') - 4; // /Jオプションがあるけど、いちおうu付けておく...

		sprintf(buff, "%u.%u.%u.%u"
			,p[0] % 90 + 10
			,p[1] % 90 + 10
			,p[2] % 90 + 10
			,p[3] % 90 + 10
			);
	}
	else
	{
		strcpy(buff, "199.0.99.99");
	}
	return buff;
}
static char *GetIdent(char *name, char *ip)
{
//	return xcout("%s @ %s", name, ip); // crypTunnelを中継したらIPは常に同じになるじゃん...
	return xcout("%s @ %s", name, GetPseudoIP(name)); // ...とりあえずnameから捏造！
}
static sint RemarkStampComp(uint v1, uint v2)
{
	Remark_t *a = (Remark_t *)v1;
	Remark_t *b = (Remark_t *)v2;

	return m_simpleComp(a->Stamp, b->Stamp);
}
static uint GetKnownNextRemarkIndex(uint64 knownStamp)
{
	Remark_t target;

	target.Stamp = knownStamp;

	return findBoundNearestRight(Remarks, (uint)&target, RemarkStampComp);
}
static uint64 GetStamp(void)
{
	return toValue64_x(makeCompactStamp(NULL));
}
static uint64 GetNextStamp(void)
{
	uint64 stamp = GetStamp();

	if (getCount(Remarks))
	{
		uint64 nxtLwStmp = ((Remark_t *)getLastElement(Remarks))->Stamp + 1ui64;

		m_maxim(stamp, nxtLwStmp);
	}
	return stamp;
}
static sint MemberLATComp_Desc(uint v1, uint v2)
{
	Member_t *a = (Member_t *)v1;
	Member_t *b = (Member_t *)v2;

	return m_simpleComp(b->LastAccessTime, a->LastAccessTime); // 降順なので b, a
}
static void UpdateMember(char *ident)
{
	Member_t *i;
	uint index;

	foreach (Members, i, index)
		if (!strcmp(i->Ident, ident))
			break;

	if (i)
	{
		i->LastAccessTime = now();
	}
	else
	{
		i = (Member_t *)memAlloc(sizeof(Member_t));
		i->LastAccessTime = now();
		i->Ident = strx(ident);

		addElement(Members, (uint)i);
	}
	rapidSort(Members, MemberLATComp_Desc);

	if (MEMBER_MAX < getCount(Members))
		ReleaseMember((Member_t *)unaddElement(Members));
}
static void PerformTh(int sock, char *ip)
{
	SockStream_t *ss = CreateSockStream(sock, 15);
	char *command;

	command = SockRecvLine(ss, COMMAND_LENMAX);
	line2JLine(command, 0, 0, 0, 0);
	cout("command: %s @ %s\n", command, GetNowStr());

	if (!strcmp(command, "GET-REMARKS"))
	{
		char *name = SockRecvLine(ss, NAME_LENMAX);
		char *ident;
		uint64 stamp;
		uint index;
		autoList_t *lines = newList();
		char *line;

		cout("GET-REMARKS.1\n");

		line2JLine(name, 1, 0, 0, 0);

		ident = GetIdent(name, ip);
		stamp = toValue64_x(SockRecvLine(ss, SVAL64_LENMAX));

		cout("ident: %s\n", ident);
		cout("stamp: %I64u\n", stamp);

		UpdateMember(ident);

		index = GetKnownNextRemarkIndex(stamp);

		cout("range: %u - %u\n", index, getCount(Remarks));

		for (; index < getCount(Remarks); index++)
		{
			Remark_t *i = (Remark_t *)getElement(Remarks, index);

			addElement(lines, (uint)xcout("%I64u", i->Stamp));
			addElement(lines, (uint)strx(i->Ident));
			addElement(lines, (uint)strx(i->Message));
			addElement(lines, 0); // Ender
		}
		addElement(lines, 0); // Ender x2

		// Remarks 読み込み中にスレッドが切り替わらないように、全て読み込んでから SockSend* する。

		foreach (lines, line, index)
		{
			if (line)
				SockSendLine_NF(ss, line);
			else
				SockSendChar(ss, 0xff); // Ender
		}
		SockFlush(ss);

		memFree(name);
		memFree(ident);
		releaseDim(lines, 1);

		cout("GET-REMARKS.2\n");
	}
	else if (!strcmp(command, "REMARK"))
	{
		char *name = SockRecvLine(ss, NAME_LENMAX);
		char *ident;
		uint64 stamp = GetNextStamp();
		char *message;
		Remark_t *i;

		cout("REMARK.1\n");

		line2JLine(name, 1, 0, 0, 0);

		ident = GetIdent(name, ip);
		message = SockRecvLine(ss, MESSAGE_LENMAX);

		line2JLine(message, 1, 0, 0, 1);
		trimEdge(message, ' ');

		cout("ident: %s\n", ident);
		cout("stamp: %I64u\n", stamp);
		cout("message: %s\n", message);

		UpdateMember(ident);

		i = (Remark_t *)memAlloc(sizeof(Remark_t));
		i->Ident = ident;
		i->Stamp = stamp;
		i->Message = message;

		addElement(Remarks, (uint)i);

		if (REMARK_MAX < getCount(Remarks))
		{
			uint index;

			for (index = 0; index < REMARK_CLEAR_NUM; index++)
				ReleaseRemark((Remark_t *)zSetElement(Remarks, index, 0));

			removeZero(Remarks);
		}
		memFree(name);

		cout("REMARK.2\n");
	}
	else if (!strcmp(command, "GET-MEMBERS"))
	{
		uint nowTime = now();
		Member_t *i;
		uint index;
		autoList_t *lines = newList();
		char *line;

		cout("GET-MEMBERS.1\n");

		foreach (Members, i, index)
			addElement(lines, (uint)xcout("%u %s", nowTime - i->LastAccessTime, i->Ident));

		// Members 読み込み中にスレッドが切り替わらないように、全て読み込んでから SockSend* する。

		foreach (lines, line, index)
			SockSendLine_NF(ss, line);

		SockSendLine_NF(ss, ""); // Ender
		SockFlush(ss);

		releaseDim(lines, 1);

		cout("GET-MEMBERS.2\n");
	}
	else
	{
		cout("Unknown command!\n");
	}

	ReleaseSockStream(ss);
	memFree(command);
}
static int IdleTh(void)
{
	while (hasKey())
	{
		if (getKey() == 0x1b)
			return 0;

		cout("ESCAPE-TO-STOP\n");
	}
	return 1;
}
int main(int argc, char **argv)
{
	uint portNo = toValue(nextArg());

	cout("へちまサーバー★ポート＝%u\n", portNo);

	cmdTitle_x(xcout("Hechima - %u", portNo));

	Remarks = newList();
	Members = newList();
	LOGPOS();
	LoadRemarks();
	LOGPOS();
	sockServerTh(PerformTh, portNo, 25, IdleTh);
	LOGPOS();
	SaveRemarks();
	LOGPOS();
	releaseDim_BR(Remarks, 1, ReleaseRemark);
	LOGPOS();
	releaseDim_BR(Members, 1, ReleaseMember);
	LOGPOS();

	cmdTitle("Hechima");
}
