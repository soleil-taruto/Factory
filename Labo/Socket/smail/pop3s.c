#include "pop3s.h"

autoList_t *GetMailList(char *popServer, uint portno, char *user, char *pass)
{
	char *outFile = makeTempPath(NULL);
	autoList_t *lines;
	char *line;
	uint index;
	autoList_t *mails = newList();

	LOGPOS();

	CheckMailCommonParams(popServer, portno, user, pass);

	mailLock();
	LOGPOS_T();

	coExecute(xcout(
		"curl pop3s://%s:%u -u %s:%s --list-only -o \"%s\""
		,popServer
		,portno
		,user
		,pass
		,outFile
		));

	LOGPOS_T();
	cout("lastSystemRet: %d\n", lastSystemRet);
	mailUnlock();

	createFileIfNotExist(outFile); // ���X�g����ł��쐬�����͂������A�O�̈�

	lines = readLines(outFile);
	trimLines(lines);

	foreach (lines, line, index)
	{
		uint mailno;
		uint64 size;
		char *p;

		if (!lineExp("<1,9,09> <1,19,09>", line))
		{
			coutJLine_x(xcout("�s���ȍs��ǂݍ��݂܂����B[%s]\n", line));
			setCount(mails, 0);
			break;
		}
		p = ne_strchr(line, ' ');
		*p = '\0';
		mailno = toValue(line);
		size = toValue(p + 1);

		if (mailno != index + 1)
		{
			cout("�s���ȃ��[���ԍ���ǂݍ��݂܂����B%u (%u)\n", mailno, index + 1);
			setCount(mails, 0);
			break;
		}
		cout("mail: %u %I64u\n", mailno, size);
		m_minim(size, UINTMAX);
		addElement(mails, (uint)size);
	}
	removeFile(outFile);
	memFree(outFile);
	releaseDim(lines, 1);
	cout("mails: %u\n", getCount(mails));
	return mails;
}
autoBlock_t *RecvMail(char *popServer, uint portno, char *user, char *pass, uint mailno, uint mailSizeMax) // mailno: 1�`
{
	char *outFile = makeTempPath(NULL);
	autoBlock_t *mail;

	LOGPOS();

	CheckMailCommonParams(popServer, portno, user, pass);
	errorCase(!m_isRange(mailno, 1, IMAX));
	errorCase(!m_isRange(mailSizeMax, 1, UINTMAX));

	mailLock();
	LOGPOS_T();

	coExecute(xcout(
		"curl pop3s://%s:%u/%u -u %s:%s -o \"%s\""
		,popServer
		,portno
		,mailno
		,user
		,pass
		,outFile
		));

	LOGPOS_T();
	cout("lastSystemRet: %d\n", lastSystemRet);
	mailUnlock();

	createFileIfNotExist(outFile); // ���[����������΍쐬����Ȃ��B-- ���[���̗L���� GetMailList() �Ŋm�F���邱�ƁB�{�֐��͉�������Ԃ��B

	{
		uint64 mailSize = getFileSize(outFile);

		if ((uint64)mailSizeMax < mailSize)
		{
			cout("��M���[�����傫�����܂��B%I64u (%u)\n", mailSize, mailSizeMax);
			createFile(outFile); // ��ɂ���B
		}
	}

	mail = readBinary(outFile);
	removeFile(outFile);
	memFree(outFile);
	cout("mail-size: %u\n", getSize(mail));
	return mail;
}
void DeleteMail(char *popServer, uint portno, char *user, char *pass, uint mailno) // mailno: 1�`
{
	LOGPOS();

	CheckMailCommonParams(popServer, portno, user, pass);
	errorCase(!m_isRange(mailno, 1, IMAX));

	mailLock();
	LOGPOS_T();

	coExecute(xcout(
		"curl pop3s://%s:%u/%u -u %s:%s --request DELE --list-only"
		,popServer
		,portno
		,mailno
		,user
		,pass
		));

	LOGPOS_T();
	cout("lastSystemRet: %d\n", lastSystemRet);
	mailUnlock();
}
