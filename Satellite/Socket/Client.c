/*
	Client.exe ���M��z�X�g�� ���M��|�[�g�ԍ� ���M�f�[�^�t�@�C�� ��M�f�[�^�t�@�C��

	- - -

	������ ... ��M�f�[�^�t�@�C���Ɏ�M�f�[�^�������o���B
	���s�� ... ��M�f�[�^�t�@�C�����쐬���Ȃ��B(���݂�����폜����)
*/

#include "C:\Factory\Common\Options\SClient.h"

static int Idle(void)
{
	return 1;
}
int main(int argc, char **argv)
{
	uchar ip[4] = { 0, 0, 0, 0 };
	char *domain;
	uint portno;
	char *rFile;
	char *wFile;
	uint tryCount;
	char *ansFile;

	domain = nextArg();
	portno = toValue(nextArg());
	rFile = nextArg();
	wFile = nextArg();

	cout("�ڑ��� = %s:%u\n", domain, portno);
	cout("���M�t�@�C�� = %s\n", rFile);
	cout("��M�t�@�C�� = %s\n", wFile);

	removeFileIfExist(wFile);

	for (tryCount = 1; ; tryCount++)
	{
		LOGPOS();
		ansFile = sockClient(ip, domain, portno, rFile, Idle);
		LOGPOS();

		cout("ansFile: %s\n", ansFile ? ansFile : "<NONE>");

		if (ansFile)
		{
			moveFile(ansFile, wFile);
			memFree(ansFile);
			break;
		}
		if (3 <= tryCount)
			break;

		coSleep(2000);
	}
	LOGPOS();
}
