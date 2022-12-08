/*
	��botsu ver

	Chat.exe [/S �T�[�o�[] [/P �|�[�g�ԍ�] [/R ���[�U�[�� ���b�Z�[�W
			| /T �J�n�X�^���v �I���X�^���v [�o�̓t�@�C��]
			| /H ���[�U�[�� [�o�̓t�@�C��]
			| /B ���[�U�[��
			| ���[�U�[��]
*/

#include "C:\Factory\Common\Options\SockClient.h"

static char *Domain = "localhost";
static uint PortNo = 59998;

static int Idle(void)
{
	return 1;
}
static char *Request(char *prmText)
{
	static uchar ip[4];
	char *prmFile = makeTempPath(NULL);
	char *ansFile;
	char *ansText;

	errorCase(!prmText);

	writeOneLineNoRet(prmFile, prmText);

	ansFile = sockClient(ip, Domain, PortNo, prmFile, Idle);

	if (ansFile)
	{
		ansText = readText(ansFile);
		removeFile(ansFile);
		memFree(ansFile);
	}
	else
	{
		ansText = NULL;
	}

	removeFile(prmFile);
	memFree(prmFile);

	return ansText;
}
static char *DoRequest(char *prmText)
{
	uint trycnt;
	char *ansText;

	for (trycnt = 1; trycnt <= 3; trycnt++)
	{
		ansText = Request(prmText);

		if (ansText)
			break;
	}
	return ansText;
}
static void DoRemark(char *userName, char *message)
{
	char *prmText;
	char *ansText;

	prmText = xcout("REMARK\n%s\n%s", userName, message);

	ansText = DoRequest(prmText);

	if (ansText && !strcmp(ansText, "REMARK_OK"))
		cout("���b�Z�[�W�𑗐M���܂����B\n");
	else
		cout("���b�Z�[�W�̑��M�Ɏ��s���܂����B\n");

	memFree(prmText);
	memFree(ansText);
}
static autoList_t *GetTimeLine(char *bgnStmp, char *endStmp)
{
	char *prmText;
	char *ansText;
	autoList_t *timeLine;

	prmText = xcout("TIME-LINE\n%s\n%s", bgnStmp, endStmp);

	ansText = DoRequest(prmText);

	if (ansText)
	{
		timeLine = tokenize(ansText, '\n');
		memFree(ansText);
		trimLines(timeLine); // �Ō����肽������..
	}
	else
	{
		timeLine = NULL;
		cout("�^�C�����C���̎擾�Ɏ��s���܂����B\n");
	}
	memFree(prmText);
	return timeLine;
}
static autoList_t *Heartbeat(char *userName)
{
	char *prmText;
	char *ansText;
	autoList_t *members;

	prmText = xcout("HEARTBEAT\n%s", userName);

	ansText = DoRequest(prmText);

	if (ansText)
	{
		members = tokenize(ansText, '\n');
		memFree(ansText);
		trimLines(members); // �Ō����肽������..
	}
	else
	{
		members = NULL;
		cout("�n�[�g�r�[�g���s\n");
	}
	memFree(prmText);
	return members;
}
static void Logout(char *userName)
{
	char *prmText;
	char *ansText;

	prmText = xcout("LOGOUT\n%s", userName);

	ansText = DoRequest(prmText);

	if (ansText && !strcmp(ansText, "LOGOUT_OK"))
		cout("���O�A�E�g����\n");
	else
		cout("���O�A�E�g�Ɏ��s���܂����B\n");

	memFree(prmText);
	memFree(ansText);
}

// ---- chat main ----

static char *UserName;
static char *LastStamp;
static uint GetTLCount;
static uint GetTLPeriod;

static void PrintRemark(char *stamp, char *userName, char *message)
{
	cout("%s (%s) %s\n", stamp, userName, message);
}
static void ChatMain(void)
{
	if (hasArgs(1))
	{
		UserName = nextArg();
	}
	else
	{
		cout("+----------------------------+\n");
		cout("| ���[�U�[������͂��ĉ����� |\n");
		cout("+----------------------------+\n");

		UserName = coInputLine();

		if (!*UserName)
		{
			cout("���[�U�[������ł��B\n");
			return;
		}
	}
	LastStamp = strx("0");

	for (; ; )
	{
		uint c;

		cmdTitle_x(xcout("Chat - ���[�U�[��=[%s] %u / %u", UserName, GetTLCount, GetTLPeriod));
		GetTLCount++;

		if (GetTLPeriod < GetTLCount)
		{
			autoList_t *timeLine = GetTimeLine(LastStamp, "Z");

			GetTLCount = 0;

			if (timeLine && getCount(timeLine))
			{
				char *line;
				uint index;

				GetTLPeriod = 0;

				foreach (timeLine, line, index)
				{
					autoList_t *tokens = tokenize(line, '\t');
					char *stamp;
					char *userName;
					char *message;

					stamp    = getLine(tokens, 0);
					userName = getLine(tokens, 1);
					message  = getLine(tokens, 2);

					PrintRemark(stamp, userName, message);

					if (index + 1 == getCount(timeLine))
					{
						memFree(LastStamp);
						LastStamp = strx(stamp);
					}
					releaseDim(tokens, 1);
				}
				releaseDim(timeLine, 1);
			}
			else
			{
				if (GetTLPeriod < 10)
					GetTLPeriod++;
			}
		}
		for (c = 5; c; c--)
		{
			if (hasKey())
			{
				char *message;

				cout("+----------------------------+\n");
				cout("| ���b�Z�[�W����͂��ĉ����� |\n");
				cout("+----------------------------+\n");

				{
					int chr = getKey();

					if (chr == 0x1b)
						goto endLoop;

					ungetKey(chr);
				}

				message = coInputLine();
				DoRemark(UserName, message);
				memFree(message);

				GetTLPeriod = 0;
			}
			sleep(300);
		}
	}
endLoop:
	cout("�I��\n");
	cmdTitle("Chat");
}

// ----

int main(int argc, char **argv)
{
readArgs:
	if (argIs("/S"))
	{
		Domain = nextArg();
		goto readArgs;
	}
	if (argIs("/P"))
	{
		PortNo = toValue(nextArg());
		goto readArgs;
	}

	errorCase(m_isEmpty(Domain));
	errorCase(!m_isRange(PortNo, 1, 65535));

	if (argIs("/R"))
	{
		char *userName;
		char *message;

		userName = nextArg();
		message = nextArg();

		DoRemark(userName, message);
		return;
	}
	if (argIs("/T"))
	{
		char *bgnStmp;
		char *endStmp;
		char *outFile;
		autoList_t *timeLine;
		char *line;
		uint index;

		bgnStmp = nextArg();
		endStmp = nextArg();

		if (hasArgs(1))
			outFile = nextArg();
		else
			outFile = NULL;

		timeLine = GetTimeLine(bgnStmp, endStmp);

		if (timeLine)
		{
			if (outFile)
			{
				writeLines(outFile, timeLine);
			}
			else
			{
				foreach (timeLine, line, index)
					cout("%s\n", line);
			}
			releaseDim(timeLine, 1);
		}
		return;
	}
	if (argIs("/H"))
	{
		char *userName;
		char *outFile;
		autoList_t *members;
		char *member;
		uint index;

		userName = nextArg();

		if (hasArgs(1))
			outFile = nextArg();
		else
			outFile = NULL;

		members = Heartbeat(userName);

		if (members)
		{
			if (outFile)
			{
				writeLines(outFile, members);
			}
			else
			{
				foreach (members, member, index)
					cout("[%s]\n", member);
			}
			releaseDim(members, 1);
		}
		return;
	}
	if (argIs("/O"))
	{
		char *userName;

		userName = nextArg();

		Logout(userName);
		return;
	}
	SockStartup();
	ChatMain();
	SockCleanup();
}
