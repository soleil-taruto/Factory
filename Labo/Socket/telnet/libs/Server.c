/*
	<PROGRAM>.exe [�Ҏ�|�[�g�ԍ�]
*/

#include "Server.h"

#define CONNECT_MAX 30
#define TIMEOUT_SEC 180
#define RECV_BUFF_MAX 2000

static uint ConnectCount;

typedef struct Info_st
{
	autoBlock_t *RecvQueue;
	autoBlock_t *SendQueue;
	uint Timeout;
	uint TSP_Prm;
}
Info_t;

static uint CreateInfo(void)
{
	Info_t *i = (Info_t *)memAlloc(sizeof(Info_t));

	ConnectCount++;

	i->RecvQueue = newBlock();
	i->SendQueue = newBlock();
	i->Timeout = now() + TIMEOUT_SEC;
	i->TSP_Prm = CreateTelnetServerPerformInfo();

	sleep(30); // ��莞�ԓ��ɂ�����ڑ����̒����B�Z���|�[�g�͊��΍�B�K�������W���[�N�T�[�r�X�Ȃ̂ł���ŗǂ��B

	return (uint)i;
}
static void ReleaseInfo(uint prm)
{
	Info_t *i = (Info_t *)prm;

	ConnectCount--;

	releaseAutoBlock(i->RecvQueue);
	releaseAutoBlock(i->SendQueue);
	ReleaseTelnetServerPerformInfo(i->TSP_Prm);

	memFree(i);
}

static char *ParseLine(autoBlock_t *buff) // ret: NULL == ���͍s����
{
	char *line = NULL;
	uint index;

	for (index = 0; index < getSize(buff); index++)
		if (getByte(buff, index) == '\n')
			break;

	if (index < getSize(buff))
	{
		line = unbindBlock2Line(desertBytes(buff, 0, index + 1));
		ucTrimEdge(line);
		toAsciiLine(line, 0, 0, 1);
	}
	return line;
}
static int Perform(int sock, uint prm)
{
	Info_t *i = (Info_t *)prm;
	char *inputLine;
	char *outputText;

	if (CONNECT_MAX < ConnectCount)
		return 0;

	if (SockRecvSequ(sock, i->RecvQueue, sockUserTransmitIndex ? 0 : 100) == -1)
		return 0;

	if (SockSendSequ(sock, i->SendQueue, 0) == -1)
		return 0;

	if (RECV_BUFF_MAX < getSize(i->RecvQueue))
		return 0;

	if (i->Timeout < now())
		return 0;

	do
	{
		if (getSize(i->SendQueue))
			break;

		inputLine = ParseLine(i->RecvQueue);
		outputText = TelnetServerPerform(inputLine, i->TSP_Prm);
		memFree(inputLine); // return 0; ������̂ł����ŊJ�����Ă����B�J�����Ă���Œl�𔻒肷��̂Œ��ӂ��邱�ƁB

		if (!outputText)
			return 0;

		ab_addLine(i->SendQueue, outputText);
		memFree(outputText);

		if (SockSendSequ(sock, i->SendQueue, 0) == -1)
			return 0;
	}
	while (inputLine);

	return 1;
}
static int Idle(void)
{
	static int endFlag;

	while (hasKey())
		if (getKey() == 0x1b)
			endFlag = 1;

	return !endFlag;
}
int main(int argc, char **argv)
{
	uint portno = 23;

	if (hasArgs(1))
		portno = toValue(nextArg());

	sockServerUserTransmit(Perform, CreateInfo, ReleaseInfo, portno, IMAX, Idle); // �ő哯���ڑ����̐����� Perform() �ł���Ă���B
}
