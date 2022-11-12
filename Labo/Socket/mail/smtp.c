#include "smtp.h"

static char *RecvLineDisp(SockStream_t *ss)
{
	char *line = SockRecvLine(ss, 1024 * 1024);

	line2JLine(line, 0, 0, 1, 1);
	cout("< %s\n", line);
	return line;
}
static uint RecvStatus(SockStream_t *ss)
{
	char *line = RecvLineDisp(ss);
	uint status;

	strchrEnd(line, ' ')[0] = '\0';
	status = toValue(line);

	memFree(line);
	return status;
}
static uint RecvEhlo(SockStream_t *ss)
{
	uint index;

	for (index = 0; index < 10000; index++)
	{
		char *line = RecvLineDisp(ss);

		if (lineExp("250 <>", line))
		{
			memFree(line);
			return 1;
		}
		memFree(line);
	}
	return 0;
}
static void SendLineDisp(SockStream_t *ss, char *line)
{
	char *p = strx(line);

	line2JLine(p, 1, 0, 0, 1);
	cout("> %s\n", p);
	memFree(p);

	SockSendLine(ss, line);
}
static void SendLineDisp_x(SockStream_t *ss, char *line)
{
	SendLineDisp(ss, line);
	memFree(line);
}

static char *MailFrom;
static char *MailTo;
static autoList_t *MailData; // ヘッダを含むメールデータ、終端の "." は含まないこと。

static int Successful;

static void Perform(int sock)
{
	SockStream_t *ss = CreateSockStream(sock, 60 * 3);
	char *line;
	uint index;

	LOGPOS_T();

	if (RecvStatus(ss) != 220)
		goto endfunc;

	SendLineDisp(ss, "EHLO localhost"); // 本当は自分のipを送るらしいよ？

	if (!RecvEhlo(ss))
		goto endfunc;

	SendLineDisp(ss, "RSET");

	if (RecvStatus(ss) != 250)
		goto endfunc;

	SendLineDisp_x(ss, xcout("MAIL FROM:%s", MailFrom));

	if (RecvStatus(ss) != 250)
		goto endfunc;

	SendLineDisp_x(ss, xcout("RCPT TO:%s", MailTo));

	if (RecvStatus(ss) != 250)
		goto endfunc;

	SendLineDisp(ss, "DATA");

	if (RecvStatus(ss) != 354)
		goto endfunc;

	foreach (MailData, line, index)
		SendLineDisp(ss, line);

	SendLineDisp(ss, ".");

	if (RecvStatus(ss) != 250)
		goto endfunc;

	SendLineDisp(ss, "QUIT");
	memFree(RecvLineDisp(ss)); // maybe 221

	Successful = 1;

endfunc:
	ReleaseSockStream(ss);
	LOGPOS_T();
}
int sendMail(char *smtpServer, uint portno, char *sendFrom, char *sendTo, autoList_t *mail) // ret: ? Successful
{
	uchar ip[4];

	cout("sendMail()\n");
	cout("smtpServer: %s\n", smtpServer);
	cout("portno: %u\n", portno);
	cout("sendFrom: %s\n", sendFrom);
	cout("sendTo: %s\n", sendTo);
	cout("mail: %u line(s)\n", getCount(mail));
	LOGPOS_T();

	MailFrom = sendFrom;
	MailTo = sendTo;
	MailData = mail;

	mailLock();

	Successful = 0;
	*(uint *)ip = 0;
	sockClientUserTransmit(ip, smtpServer, portno, Perform);

	mailUnlock();

	LOGPOS_T();
	cout("send mail condition: %d\n", Successful);
	return Successful;
}
int sendMailEx(char *smtpServer, uint portno, char *sendFrom, char *sendTo, autoList_t *mail, uint retrycnt, uint retryWaitMillis) // ret: ? Successful
{
	cout("sendMailEx()\n");

	for (; ; retrycnt--)
	{
		if (sendMail(smtpServer, portno, sendFrom, sendTo, mail))
		{
			return 1;
		}
		cout("sendMail() Fault... retrycnt: %u\n", retrycnt);

		if (!retrycnt)
		{
			return 0;
		}
		coSleep(retryWaitMillis);
	}
}
int sendMailEx2(char *smtpServer, uint portno, char *sendFrom, char *sendTo, autoList_t *mail)
{
	return sendMailEx(smtpServer, portno, sendFrom, sendTo, mail, 15, 20000);
}
