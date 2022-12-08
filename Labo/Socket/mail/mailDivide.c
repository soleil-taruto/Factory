/*
	Forward Table ファイル

	ptn ptn ptn        受信したメールの From に含まれるパターンのリスト。ptn == * は全てに一致
		forward-mail   転送先のメールアドレス
		forward-mail
		forward-mail
	/

	これの繰り返し
*/

#include "pop3.h"
#include "smtp.h"
#include "tools.h"

static char *PopServer;
static uint PopPortno = 110;
static char *SmtpServer;
static uint SmtpPortno = 25;
static char *PopUserName;
static char *PopPassphrase;
static char *SelfMailAddress;
static autoList_t *ForwardTable;
static autoList_t *FromPatternsList;
static int KeepSender;

#define WILDCARD "*"

static autoList_t *MakeMailData(char *destAddr, autoList_t *mail)
{
	autoList_t *sendData = newList();

#define ADD_HEADER(hdr_name, hdr_value) \
	addElement(sendData, (uint)xcout("%s: %s", hdr_name, hdr_value))

#define CPY_HEADER(hdr_name) \
	ADD_HEADER(hdr_name, c_RefMailHeader(mail, hdr_name))

	CPY_HEADER("Date");
	if (KeepSender) CPY_HEADER("From");
	else            ADD_HEADER("From", SelfMailAddress);
	ADD_HEADER("To", destAddr);
	CPY_HEADER("Subject");
	ADD_HEADER("Message-Id", c_MakeMailMessageID(SelfMailAddress));
	CPY_HEADER("MIME-Version");
	CPY_HEADER("Content-Type");
	CPY_HEADER("Content-Transfer-Encoding");
	ADD_HEADER("X-Mailer", "md");

	addElement(sendData, (uint)strx(""));
	addLines_x(sendData, GetMailBody(mail));

	return sendData;
}
static void RecvEvent(autoList_t *mail)
{
	char *from = GetMailHeader(mail, "From");
	char *ptns;
	char *ptn;
	uint index;
	autoList_t *mailAddrList;
	char *mailAddr;

	if (!from)
		return;

	foreach (FromPatternsList, ptns, index)
	{
		autoList_t *ptnList = ucTokenize(ptns);
		uint ptnndx;

		foreach (ptnList, ptn, ptnndx)
		{
			cout("ptn: %s\n", ptn);

			if (strstr(from, ptn) || !strcmp(ptn, WILDCARD))
			{
				goto found;
			}
		}
		releaseDim(ptnList, 1);
	}
	found:
	memFree(from);
	cout("ptn, index: %p, %u\n", ptn, index);

	if (!ptn) // Not found ptn.
		return;

	mailAddrList = (autoList_t *)getElement(ForwardTable, index);
	cout("forward num: %u\n", getCount(mailAddrList));

	foreach (mailAddrList, mailAddr, index)
	{
		autoList_t *mail2;

		cout("forward addr: %s\n", mailAddr);

		mail2 = MakeMailData(mailAddr, mail);
		sendMailEx2(SmtpServer, SmtpPortno, SelfMailAddress, mailAddr, mail2);
		releaseDim(mail2, 1);
	}
	cout("forward end.\n");
}
static void RecvLoop(void)
{
	for (; ; )
	{
		uint index;

		do
		{
			autoList_t *mails = mailRecv(PopServer, PopPortno, PopUserName, PopPassphrase, 3, 1024 * 1024 * 64, 1);
			autoList_t *mail;

			foreach (mails, mail, index)
			{
				/*
					即返信すると破棄されることがある？
				*/
//				coSleep(3000); moved @ 2016.6.28

				RecvEvent(mail);
			}
			releaseDim(mails, 2);
		}
		while (index);

		for (index = 9; index; index--)
		{
			cout("%u\n", index);
			coSleep(3000);

			while (hasKey())
			{
				switch (getKey())
				{
				case 0x1b:
					goto endLoop;

				case 0x20:
					goto endWait;
				}
			}
		}
	endWait:
		cout("End wait.\n");
	}
endLoop:;
}
int main(int argc, char **argv)
{
	ForwardTable = newList();

readArgs:
	if (argIs("/PD")) // Pop server Domain
	{
		PopServer = nextArg();
		goto readArgs;
	}
	if (argIs("/PP")) // Pop server Port number
	{
		PopPortno = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/SD")) // Smtp server Domain
	{
		SmtpServer = nextArg();
		goto readArgs;
	}
	if (argIs("/SP")) // Smtp server Port number
	{
		SmtpPortno = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/U")) // User name
	{
		PopUserName = nextArg();
		goto readArgs;
	}
	if (argIs("/P")) // Passphrase
	{
		PopPassphrase = nextArg();
		goto readArgs;
	}
	if (argIs("/M")) // self Mail-address
	{
		SelfMailAddress = nextArg();
		goto readArgs;
	}
	if (argIs("/FT")) // Forward Table
	{
		addElements_x(ForwardTable, readResourceLinesList(nextArg()));
		goto readArgs;
	}
	if (argIs("/KS")) // Keep Sender
	{
		KeepSender = 1;
		goto readArgs;
	}

	errorCase(m_isEmpty(PopServer));
	errorCase(!PopPortno || 0xffff < PopPortno);
	errorCase(m_isEmpty(SmtpServer));
	errorCase(!SmtpPortno || 0xffff < SmtpPortno);
	errorCase(m_isEmpty(PopUserName));
	errorCase(m_isEmpty(PopPassphrase));
	errorCase(m_isEmpty(SelfMailAddress));
	errorCase(!getCount(ForwardTable));

	FromPatternsList = discerpHeaders(ForwardTable);

	cmdTitle("mailDivide");

	SockStartup();
	RecvLoop();
	SockCleanup();

	cout("\\e\n");
}
