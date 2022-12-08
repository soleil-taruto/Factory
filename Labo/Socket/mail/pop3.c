#include "pop3.h"

static char *RecvLineDisp(SockStream_t *ss)
{
	char *line = SockRecvLine(ss, 1024 * 1024);

	line2JLine(line, 0, 0, 1, 1);
	cout("> %s\n", line);
	return line;
}
static char *RecvBinLineDisp(SockStream_t *ss)
{
	char *line = SockRecvLine(ss, 1024 * 1024);
	char *p;

	p = strx(line);
	line2JLine(p, 1, 0, 0, 1);
	cout("* %s\n", p);
	memFree(p);
	return line;
}
static void SendLineDisp(SockStream_t *ss, char *line)
{
	cout("< %s\n", line);
	SockSendLine(ss, line);
}
static void SendLineDisp_x(SockStream_t *ss, char *line)
{
	SendLineDisp(ss, line);
	memFree(line);
}

static char *UserName;
static char *Passphrase;
static uint RecvNumMax;
static uint MailSizeMax;
static int RecvAndDeleteMode;
static autoList_t *MailList;

static autoList_t *RecvMail(SockStream_t *ss, uint mailno)
{
	autoList_t *lines = newList();
	char *line;
	uint szCount = 0;
	uint overCount = 0;

	SendLineDisp_x(ss, xcout("RETR %u", mailno));
	memFree(RecvLineDisp(ss)); // maybe +OK <size> octets

	for (; ; )
	{
		if (IsEOFSockStream(ss))
		{
			cout("endStream\n");
			overCount = 1;
			break;
		}
		line = RecvBinLineDisp(ss);

		if (!strcmp(line, ".")) // ? メール本体の終了
		{
			memFree(line);
			break;
		}
		szCount += strlen(line);
		szCount += 1 + 4; // null char + element of lines

		if (overCount || MailSizeMax < szCount)
		{
			memFree(line);

			overCount++;
			cout("overCount: %u, %f\n", overCount, overCount / (double)IMAX);

			if (IMAX < overCount) // もう諦める。
				break;
		}
		else
		{
			addElement(lines, (uint)line);
		}
	}
	cout("szCount: %u\n", szCount);

	if (overCount)
	{
		releaseDim(lines, 1);
		lines = NULL;
	}
	return lines;
}
static void Perform(int sock)
{
	SockStream_t *ss = CreateSockStream(sock, 60 * 3);
	char *line;
	autoList_t *tokens;
	uint mailnum;
	uint mailno;

	LOGPOS_T();

	memFree(RecvLineDisp(ss)); // maybe +OK

	SendLineDisp_x(ss, xcout("USER %s", UserName));
	memFree(RecvLineDisp(ss)); // maybe +OK

	SendLineDisp_x(ss, xcout("PASS %s", Passphrase));
	memFree(RecvLineDisp(ss)); // maybe +OK

	SendLineDisp(ss, "STAT");
	line = RecvLineDisp(ss); // maybe +OK <mail-num> <total-size>
	tokens = tokenize(line, ' ');

	if (!strcmp("+OK", refLine(tokens, 0)))
	{
		mailnum = toValue(refLine(tokens, 1));
	}
	else
	{
		mailnum = 0;
	}
	memFree(line);
	releaseDim(tokens, 1);

	cout("mailnum: %u\n", mailnum);

	for (mailno = 1; mailno <= mailnum; mailno++)
	{
		autoList_t *mail = RecvMail(ss, mailno);

		if (RecvAndDeleteMode) // 存在するけど受信できないメールがあった場合それを削除するために break; より前に DELE を実行する。
		{
			SendLineDisp_x(ss, xcout("DELE %u", mailno));
			memFree(RecvLineDisp(ss)); // maybe +OK
		}
		if (!mail)
			break;

		addElement(MailList, (uint)mail);
	}
	SendLineDisp(ss, "QUIT");
	memFree(RecvLineDisp(ss)); // maybe +OK

	LOGPOS_T();

	ReleaseSockStream(ss);
}
autoList_t *mailRecv(char *popServer, uint portno, char *user, char *pass, uint recvMax, uint sizeMax, int recvAndDelete) // ret: releaseDim(mailRecv(), 2);
{
	uchar ip[4];

	cout("mailRecv()\n");
	cout("popServer: %s\n", popServer);
	cout("portno: %u\n", portno);
	cout("user: %s\n", user);
	cout("pass: %s\n", pass);
	cout("recvMax: %u\n", recvMax);
	cout("sizeMax: %u\n", sizeMax);
	cout("recvAndDelete: %d\n", recvAndDelete);
	LOGPOS_T();

	UserName = user;
	Passphrase = pass;
	RecvNumMax = recvMax;
	MailSizeMax = sizeMax;
	RecvAndDeleteMode = recvAndDelete;
	MailList = newList();

	mailLock();

	*(uint *)ip = 0;
	sockClientUserTransmit(ip, popServer, portno, Perform);

	mailUnlock();

	LOGPOS_T();
	cout("%u mail(s) recv.\n", getCount(MailList));
	return MailList;
}
