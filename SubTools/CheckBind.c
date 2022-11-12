#include "C:\Factory\Common\Options\SockServer.h"

static int Perform(char *prmFile, char *ansFile)
{
	return 0;
}
static void ErrorFnlz(void)
{
	cout("指定されたポート番号は使用できません。ret=1\n");

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

	cout("ポート番号 [%u]\n", portno);

	addFinalizer(ErrorFnlz);
	sockServer(Perform, portno, 1, 0, Idle);
	unaddFinalizer(ErrorFnlz);

	cout("指定されたポート番号は使用できます。ret=0\n");

	termination(0);
}
