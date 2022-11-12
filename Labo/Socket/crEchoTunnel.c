/*
	�N���C�A���g���Ő��������p�^�[�����T�[�o�[���Ԃ���Ƃ������Ƃ́A
	�ߋ��̉���f�[�^��͕킵�Ă���킯�ł͂Ȃ��̂ŃT�[�o�[��M�p�ł���B
	�M�p�ł���T�[�o�[�Ƃ������Ƃ́A�T�[�o�[�����������J�E���^���M�p�ł���̂ŁA
	�ȍ~�̒ʐM���M�p�ł���B�Ƃ��������B

	-> �ʐM�̓r������n�C�W���b�N���ꂽ��ʖڂ���Ȃ����B
*/

#include "C:\Factory\Labo\Socket\tunnel\libs\Tunnel.h"
#include "C:\Factory\Common\Options\CryptoRand.h"

#define SENDRECV_TIMEOUT_MILLIS 2000
#define ECHOPTN_SIZE 16

static int ServerMode;

static void Perform(int sock, int fwdSock)
{
	int successful = 0;

	if (ServerMode)
	{
		autoBlock_t *data = newBlock();

		successful =
			SockRecvSequLoop(sock, data, SENDRECV_TIMEOUT_MILLIS, ECHOPTN_SIZE) == ECHOPTN_SIZE &&
			SockSendSequLoop(sock, data, SENDRECV_TIMEOUT_MILLIS) == ECHOPTN_SIZE;

		releaseAutoBlock(data);
	}
	else
	{
		autoBlock_t *data = makeCryptoRandBlock(ECHOPTN_SIZE);
		autoBlock_t *bkData;

		bkData = copyAutoBlock(data);

		successful =
			SockSendSequLoop(fwdSock, data, SENDRECV_TIMEOUT_MILLIS) == ECHOPTN_SIZE &&
			SockRecvSequLoop(fwdSock, data, SENDRECV_TIMEOUT_MILLIS, ECHOPTN_SIZE) == ECHOPTN_SIZE &&
			isSameBlock(data, bkData);

		releaseAutoBlock(data);
		releaseAutoBlock(bkData);
	}
	if (!successful)
		return;

	CrossChannel(sock, fwdSock, NULL, 0, NULL, 0);
}
static int ReadArgs(void)
{
	if (argIs("/R"))
	{
		ServerMode = 1;
		return 1;
	}
	return 0;
}
static char *GetTitleSuffix(void)
{
	return xcout("[%s]", ServerMode ? "Server" : "Client");
}
int main(int argc, char **argv)
{
	TunnelMain(ReadArgs, Perform, "crEchoTunnel", GetTitleSuffix);
}
