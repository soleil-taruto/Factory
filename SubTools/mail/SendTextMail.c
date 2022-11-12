/*
	SendTextMail.exe [/E] /S �T�[�o�[ [/P �|�[�g�ԍ�] /U ���[�U�[�� �p�X���[�h /MF ���M�����[���A�h���X /MT ���M�惁�[���A�h���X /T �\�� [/L �e�L�X�g�s | /F �e�L�X�g�t�@�C��]...

		/E ... env ����ݒ��ǂݍ���

			SMTPS_SERVER      ... �T�[�o�[
			SMTPS_SERVER_PORT ... �|�[�g�ԍ�
			SMTPS_USER        ... ���[�U�[��
			SMTPS_PASS        ... �p�X���[�h
			SMTPS_FROM        ... ���M�����[���A�h���X
			SMTPS_TO          ... ���M�惁�[���A�h���X

	----
	��F���M�惁�[���A�h���X�ȊO�� env ����ǂݍ���ŁA���M�惁�[���A�h���X�͈����Ŏw�肵�����ꍇ

	SendTextMail.exe /E /MT aaaaaaaa@example.test /T �\�� /F �{��.txt

		/E �� env ����ǂݍ��񂾒l�͈ȍ~�̃I�v�V�����ɂ���ď㏑���\�ł��B
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\UTF.h"
#include "C:\Factory\Labo\Socket\smail\smtps.h"

static char *SmtpServer;
static uint Portno = 465;
static char *User;
static char *Pass;
static char *MailAddressFrom;
static char *MailAddressTo;
static char *Subject;
static autoList_t *BodyLines;

static char *MakeDateField(void)
{
	stampData_t tmpsd = *getStampDataTime(0L);

	return xcout(
		"%s, %u %s %04u %02u:%02u:%02u +0900"
		,getEWeekDay(tmpsd.weekday)
		,tmpsd.day
		,getEMonth(tmpsd.month)
		,tmpsd.year
		,tmpsd.hour
		,tmpsd.minute
		,tmpsd.second
		);
}
static char *ToQuotedPrintable(char *line)
{
	char *file = makeTempPath(NULL);
	autoBlock_t *text_u;
	char *ret;

	writeOneLineNoRet_b(file, line);
	SJISToUTF8File(file, file);
	text_u = readBinary(file);

	{
		autoBlock_t *buff = newBlock();
		uint index;

		ab_addLine(buff, "=?UTF-8?Q?");

		for (index = 0; index < getSize(text_u); index++)
		{
			ab_addChar(buff, '=');
			ab_addLine_x(buff, xcout("%02X", b_(text_u)[index]));
		}
		ab_addLine(buff, "?=");

		ret = unbindBlock2Line(buff);
	}

	releaseAutoBlock(text_u);
	removeFile(file);
	memFree(file);
	return ret;
}
static char *ToUTF8Base64(autoList_t *lines)
{
	char *file = makeTempPath(NULL);
	autoBlock_t *text_u;
	char *ret;

	writeLines(file, lines);
	SJISToUTF8File(file, file);
	text_u = readBinary(file);
	ret = unbindBlock2Line(encodeBase64(text_u));
	releaseAutoBlock(text_u);
	removeFile(file);
	memFree(file);
	return ret;
}
static void SendTextMail(void)
{
	autoList_t *lines = newList();

	{
		char *dateField = MakeDateField();
		char *messageId = MakeMailMessageID(MailAddressFrom);
		char *qpSubject = ToQuotedPrintable(Subject);

		addElement(lines, (uint)xcout("Date: %s", dateField));
		addElement(lines, (uint)xcout("From: %s", MailAddressFrom));
		addElement(lines, (uint)xcout("To: %s", MailAddressTo));
		addElement(lines, (uint)xcout("Subject: %s", qpSubject));
		addElement(lines, (uint)xcout("Message-Id: %s", messageId));
		addElement(lines, (uint)strx("MIME-Version: 1.0"));
		addElement(lines, (uint)strx("Content-Type: text/plain; charset=\"UTF-8\""));
		addElement(lines, (uint)strx("Content-Transfer-Encoding: base64"));
		addElement(lines, (uint)strx("X-Mailer: stm"));
		addElement(lines, (uint)strx(""));
		addElement(lines, (uint)ToUTF8Base64(BodyLines));

		memFree(dateField);
		memFree(messageId);
		memFree(qpSubject);
	}

	{
		autoBlock_t *mail = ab_fromLine_x(untokenize_xc(lines, "\r\n"));

		SendMail(SmtpServer, Portno, User, Pass, MailAddressFrom, MailAddressTo, mail);

		releaseAutoBlock(mail);
	}
}
int main(int argc, char **argv)
{
	BodyLines = newList();

readArgs:
	if (argIs("/S"))
	{
		SmtpServer = nextArg();
		goto readArgs;
	}
	if (argIs("/P"))
	{
		Portno = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/U"))
	{
		User = nextArg();
		Pass = nextArg();
		goto readArgs;
	}
	if (argIs("/MF"))
	{
		MailAddressFrom = nextArg();
		goto readArgs;
	}
	if (argIs("/MT"))
	{
		MailAddressTo = nextArg();
		goto readArgs;
	}
	if (argIs("/T"))
	{
		Subject = nextArg();
		goto readArgs;
	}
	if (argIs("/L"))
	{
		addElement(BodyLines, (uint)strx(nextArg()));
		goto readArgs;
	}
	if (argIs("/F"))
	{
		addElements_x(BodyLines, readLines(nextArg()));
		goto readArgs;
	}
	if (argIs("/E"))
	{
		SmtpServer      = getAppDataEnv("SMTPS_SERVER",        SmtpServer);
		Portno          = getAppDataEnv32("SMTPS_SERVER_PORT", Portno);
		User            = getAppDataEnv("SMTPS_USER",          User);
		Pass            = getAppDataEnv("SMTPS_PASS",          Pass);
		MailAddressFrom = getAppDataEnv("SMTPS_FROM",          MailAddressFrom);
		MailAddressTo   = getAppDataEnv("SMTPS_TO",            MailAddressTo);
		goto readArgs;
	}

	CheckMailServer(SmtpServer);
	CheckMailPortno(Portno);
	CheckMailUser(User);
	CheckMailPass(Pass);
	CheckMailAddress(MailAddressFrom);
	CheckMailAddress(MailAddressTo);
	errorCase(m_isEmpty(Subject));
	// BodyLines

	SendTextMail();
}
