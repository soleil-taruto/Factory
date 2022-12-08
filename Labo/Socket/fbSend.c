/*
	fbSend.exe <DOMAIN>:<PORTNO>/<URL-PHRASE> <FILE>
	fbSend.exe <DOMAIN>:<PORTNO> <FILE>
	fbSend.exe <DOMAIN> <FILE>
	fbSend.exe <DOMAIN>/<URL-PHRASE> <FILE>
*/

#include "C:\Factory\Common\Options\SockClient.h"

#define UFBUFF_SIZE (256 * 1024)

static char *URLPhrase = "otankonasu";
static char *UpFile;

static int SUFSuccessful;

static void SendUpFile(int sock)
{
	autoBlock_t *buffer = newBlock();
	FILE *fp;

	ab_addLine(buffer, "!");
	ab_addLine(buffer, URLPhrase);
	ab_addLine(buffer, "\n"); // URLPhrase �̖����� '\r' �͂Ȃ����낤�A�A�A

	cout("Header\n");

	while (getSize(buffer))
		if (SockSendSequ(sock, buffer, 3000) == -1)
			return;

	cout("Header End\n");

	// �_�E�����[�h���̑��̃N���C�A���g�����s�A���~������B���̕ӂ͓K��
	cout("Wait\n");
	Sleep(5000);
	cout("Wait End\n");

	fp = fileOpen(UpFile, "rb");

	for (; ; )
	{
		if (getSize(buffer) < UFBUFF_SIZE)
			ab_addBytes_x(buffer, readBinaryBlock(fp, UFBUFF_SIZE));

		if (getSize(buffer) == 0)
			break;

		if (SockSendSequ(sock, buffer, 1) == -1)
			return;

		if (pulseSec(10, NULL))
		{
			char *strw = xcout("%I64d", _ftelli64(fp));
			char *strx;

			strw = thousandComma(strw);

			cout("%s �������� %s �o�C�g���炢���M���܂����B\n", strx = makeStamp(0), strw);

			memFree(strw);
			memFree(strx);
		}
	}
	fileClose(fp);
	SUFSuccessful = 1;
}
int main(int argc, char **argv)
{
	uchar ip[4];
	char *domain;
	char *p;
	uint portno = 80;

	domain = strx(nextArg());

	if (p = strchr(domain, '/'))
	{
		*p = '\0';
		URLPhrase = p + 1;
	}
	if (p = strchr(domain, ':'))
	{
		*p = '\0';
		portno = toValue(p + 1);
	}
	cout("�I: %s\n", domain);
	cout("�`: %u\n", portno);
	cout("URL-phrase: %s\n", URLPhrase);

	if (hasArgs(1))
	{
		UpFile = makeFullPath(nextArg());
		cout("�t�@�C��: %s\n", UpFile);
		errorCase(!existFile(UpFile));
	}
	else
	{
		UpFile = dropFile();
	}

	*(uint *)ip = 0;
	sockClientUserTransmit(ip, domain, portno, SendUpFile);

	if (!SUFSuccessful)
		error();

	cout("���M����\n");
}
