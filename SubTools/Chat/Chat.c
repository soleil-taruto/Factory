/*
	Chat.exe [/S �T�[�o�[] [/P �|�[�g�ԍ�] (/R �^�C���X�^���v ���b�Z�[�W
			| /T �J�n�X�^���v �I���X�^���v [�o�̓t�@�C��]
			| /H ���ʖ� ���b�Z�[�W [�o�̓t�@�C��]
			| /O ���ʖ�
			| /B ���b�Z�[�W SPEED_PML TONE_PML VOLUME_PML VOICE)

	/R ... ����
	/T ... �^�C�����C���擾
	/H ... �n�[�g�r�[�g�E���O�C��
	/O ... ���O�A�E�g
	/B ... �_�ǂ݂����Ƀ��b�Z�[�W�𑗂�B

	���ʖ�         ... �󕶎���s��
	�^�C���X�^���v ... YYYYMMDDhhmmss �`��, �K���ȕ�������w�肷��ƁA�T�[�o�[�̌������ɂȂ�B�󕶎���s��
	�J�n�X�^���v   ... YYYYMMDDhhmmss �`��
	�I���X�^���v   ... YYYYMMDDhhmmss �`��
	���b�Z�[�W     ... �󕶎���s��
	�o�̓t�@�C��   ... �ȗ�����ƁA�W���o��
*/

#include "C:\Factory\Common\Options\SockClient.h"
#include "Bouyomichan.h"

static char *Domain = "localhost";
static uint PortNo = 59999;

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
		ansText = readText_b(ansFile);
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

// ---- requests ----

static void DoRemark(char *stamp, char *message)
{
	char *prmText;
	char *ansText;

	prmText = xcout("REMARK\n%s\n%s", stamp, message);
	ansText = DoRequest(prmText);

	if (ansText && !strcmp(ansText, "REMARK_OK"))
		cout("���b�Z�[�W�𑗐M���܂����B\n");
	else
		cout("���b�Z�[�W�̑��M�Ɏ��s���܂����B\n");

	memFree(prmText);
	memFree(ansText);
}
static char *GetTimeLine(char *bgnStmp, char *endStmp)
{
	char *prmText;
	char *ansText;

	prmText = xcout("TIME-LINE\n%s\n%s", bgnStmp, endStmp);
	ansText = DoRequest(prmText);

	if (ansText)
		cout("�^�C�����C�����擾���܂����B\n");
	else
		cout("�^�C�����C���̎擾�Ɏ��s���܂����B\n");

	memFree(prmText);
	return ansText;
}
static char *Heartbeat(char *ident, char *message)
{
	char *prmText;
	char *ansText;

	prmText = xcout("HEARTBEAT\n%s\n%s", ident, message);
	ansText = DoRequest(prmText);

	if (ansText)
		cout("�n�[�g�r�[�g�𑗐M���܂����B\n");
	else
		cout("�n�[�g�r�[�g�̑��M�Ɏ��s���܂����B\n");

	memFree(prmText);
	return ansText;
}
static void Logout(char *userName)
{
	char *prmText;
	char *ansText;

	prmText = xcout("LOGOUT\n%s", userName);
	ansText = DoRequest(prmText);

	if (ansText && !strcmp(ansText, "LOGOUT_OK"))
		cout("���O�A�E�g���܂����B\n");
	else
		cout("���O�A�E�g�Ɏ��s���܂����B\n");

	memFree(prmText);
	memFree(ansText);
}
static sint ServerTimeDiff(void)
{
	char *prmText;
	char *ansText;
	sint serverTimeDiff = 0;

	prmText = xcout("SERVER-TIME-DIFF\n%s", c_makeCompactStamp(NULL));
	ansText = DoRequest(prmText);

	if (ansText)
		serverTimeDiff = atoi(ansText);

	cout("serverTimeDiff: %d\n", serverTimeDiff);

	memFree(prmText);
	memFree(ansText);
	return serverTimeDiff;
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
		char *stamp;
		char *message;

		stamp = nextArg();
		message = nextArg();

		DoRemark(stamp, message);
		return;
	}
	if (argIs("/T"))
	{
		char *bgnStmp;
		char *endStmp;
		char *outFile;
		char *timeLine;

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
				writeOneLineNoRet_b(outFile, timeLine);
			else
				cout("%s", timeLine);

			memFree(timeLine);
		}
		return;
	}
	if (argIs("/H"))
	{
		char *ident;
		char *message;
		char *outFile;
		char *members;

		ident = nextArg();
		message = nextArg();

		if (hasArgs(1))
			outFile = nextArg();
		else
			outFile = NULL;

		members = Heartbeat(ident, message);

		if (members)
		{
			if (outFile)
				writeOneLineNoRet_b(outFile, members);
			else
				cout("%s", members);

			memFree(members);
		}
		return;
	}
	if (argIs("/O"))
	{
		char *ident;

		ident = nextArg();

		Logout(ident);
		return;
	}
	if (argIs("/B"))
	{
		char *message;
		uint speedPml;
		uint tonePml;
		uint volumePml;
		uint voice;

		message   = nextArg();
		speedPml  = toValue(nextArg());
		tonePml   = toValue(nextArg());
		volumePml = toValue(nextArg());
		voice     = toValue(nextArg());

		SendToBouyomichan(Domain, PortNo, message, speedPml, tonePml, volumePml, voice);
		return;
	}
	if (argIs("/STD")) // server time diff
	{
		char *outFile = nextArg();
		sint ret = ServerTimeDiff();
		char *sRet;

		sRet = xcout("%d", ret);
		writeOneLineNoRet_b(outFile, sRet);
		memFree(sRet);
		return;
	}
}
