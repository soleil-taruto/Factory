#include "C:\Factory\Common\Options\SockServer.h"

static int Perform(char *prmFile, char *ansFile)
{
	return 0;
}
static void ErrorFnlz(void)
{
	cout("�w�肳�ꂽ�|�[�g�ԍ��͎g�p�ł��܂���Bret=1\n");

	termination(1);
}
static int Idle(void)
{
	return 0;
}
int main(int argc, char **argv)
{
	uint portno = 80;

	if (hasArgs(1))
		portno = toValue(nextArg());

	cout("�|�[�g�ԍ� [%u]\n", portno);

	addFinalizer(ErrorFnlz);
	sockServer(Perform, portno, 1, 0, Idle);
	unaddFinalizer(ErrorFnlz);

	cout("�w�肳�ꂽ�|�[�g�ԍ��͎g�p�ł��܂��Bret=0\n");

	termination(0);
}
