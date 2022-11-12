/*
	Server.exe ��~�C�x���g�� ��M�|�[�g�ԍ� ���M��-Fortewave-Ident ��M��-Fortewave-Ident

	- - -

	�I�����@

		�G�X�P�[�v�L�[���������A���O�t���C�x���g�u��~�C�x���g���v���Z�b�g����B

	�^�C���A�E�g

		�����B
*/

#include "C:\Factory\Common\Options\SockServer.h"
#include "C:\Factory\Satellite\libs\Flowertact\Fortewave.h"

static uint EvStop;
static Frtwv_t *SendPipe;
static Frtwv_t *RecvPipe;

static int StopFlag;

static void CheckStop(void)
{
	if (handleWaitForMillis(EvStop, 0))
	{
		cout("��~�C�x���g���E���܂����B");
		StopFlag = 1;
	}
}
static int Perform(char *prmFile, char *ansFile)
{
	autoBlock_t *prmData;
	autoBlock_t *ansData;
	int ret = 0;

	LOGPOS();

	if (StopFlag)
		goto endFunc;

	prmData = readBinary(prmFile);

	cout("> %u\n", getSize(prmData));

	Frtwv_Clear(SendPipe);
	Frtwv_Clear(RecvPipe);
	Frtwv_Send(SendPipe, prmData);

	releaseAutoBlock(prmData);

	for (; ; )
	{
		ansData = Frtwv_Recv(RecvPipe, 2000);

		if (ansData)
		{
			cout("< %u\n", getSize(ansData));

			writeBinary(ansFile, ansData);
			releaseAutoBlock(ansData);
			ret = 1;
			break;
		}
		CheckStop();

		if (StopFlag)
			break;
	}
endFunc:
	LOGPOS();
	return ret;
}
int Idle(void)
{
	int ret = 1;

	while (hasKey())
		if (getKey() == 27) // escape
			ret = 0;

	CheckStop();

	if (StopFlag)
		ret = 0;

	return ret;
}
int main(int argc, char **argv)
{
	char *evStopName;
	uint portno;
	char *sendIdent;
	char *recvIdent;

	evStopName = nextArg();
	portno = toValue(nextArg());
	sendIdent = nextArg();
	recvIdent = nextArg();

	cout("��~�C�x���g�� = %s\n", evStopName);
	cout("�Ҏ�|�[�g = %u\n", portno);
	cout("���M-IDENT = %s\n", sendIdent);
	cout("��M-IDENT = %s\n", recvIdent);

	LOGPOS();

	EvStop = eventOpen(evStopName);
	SendPipe = Frtwv_Create(sendIdent);
	RecvPipe = Frtwv_Create(recvIdent);

	sockServer(Perform, portno, 20, 2100000, Idle);

	handleClose(EvStop);
	Frtwv_Release(SendPipe);
	Frtwv_Release(RecvPipe);

	LOGPOS();
}
