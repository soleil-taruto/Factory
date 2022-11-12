/*
	ServiceMain()
		����
			�Ȃ�

		�߂�l
			�Ȃ�

	HttIPFile
		HTT_IP_FILE �̃t���p�X

	HttRecvFile
		HTT_RECV_FILE �̃t���p�X

	HttSendFile
		HTT_SEND_FILE �̃t���p�X

	HttTightFile
		HTT_TIGHT_FILE �̃t���p�X

	HttIP
		�N���C�A���g��IP�A�h���X

	BootTime
		���̃v���Z�X���N�����������i�������j

	ConnectTime
		�ڑ���������

	LastRecvTime
		�Ō�Ɏ�M��������

	LastSendTime
		�Ō�ɑ��M��������

	ConnectElapseTime
		�ڑ����Ă���̌o�ߎ���

	LastRecvElapseTime
		�Ō�Ɏ�M���Ă���̌o�ߎ���

	LastSendElapseTime
		�Ō�ɑ��M���Ă���̌o�ߎ���
*/

#include "Service.h"

char *HttIPFile;
char *HttRecvFile;
char *HttSendFile;
char *HttTightFile;
char *HttIP;

time_t BootTime;
time_t ConnectTime;
time_t LastRecvTime;
time_t LastSendTime;
time_t ConnectElapseTime;
time_t LastRecvElapseTime;
time_t LastSendElapseTime;

void ServiceDisconnect(void)
{
	LOGPOS();
	removeFile(HttSendFile);
	termination(0);
}
static void ServiceErrorDisconnectExit(void)
{
	if (errorOccurred)
	{
		LOGPOS();
		removeFile(HttSendFile);
		termination(1);
	}
}
int main(int argc, char **argv)
{
	HttIPFile = makeFullPath(HTT_IP_FILE);
	HttRecvFile = makeFullPath(HTT_RECV_FILE);
	HttSendFile = makeFullPath(HTT_SEND_FILE);
	HttTightFile = makeFullPath(HTT_TIGHT_FILE);

	if (!existFile(HttIPFile) || !existFile(HttRecvFile) || !existFile(HttSendFile))
	{
		LOGPOS();
		termination(0);
	}
	addFinalizer(ServiceErrorDisconnectExit);

	BootTime = time(NULL);

	if (getFileSize(HttIPFile) != 0) // ����M���C��
	{
		HttIP = readFirstLine(HttIPFile);

		ConnectTime = getFileModifyTime(HttIPFile);
		LastRecvTime = getFileModifyTime(HttRecvFile);
		LastSendTime = getFileModifyTime(HttSendFile);
		ConnectElapseTime = BootTime - ConnectTime;
		LastRecvElapseTime = BootTime - LastRecvTime;
		LastSendElapseTime = BootTime - LastSendTime;
cout("ConnectElapseTime: %I64d\n", ConnectElapseTime); // test
cout("LastRecvElapseTime: %I64d\n", LastRecvElapseTime); // test
cout("LastSendElapseTime: %I64d\n", LastSendElapseTime); // test

		ServiceMain();
	}
	else // ����I�ȋ���s
	{
		HttIP = strx("0.0.0.0");

		ConnectTime = BootTime;
		LastRecvTime = BootTime;
		LastSendTime = BootTime;
		ConnectElapseTime = 0;
		LastRecvElapseTime = 0;
		LastSendElapseTime = 0;

		ServicePeriod();
	}
	termination(0);
}
