#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	if (hasArgs(2))
	{
		cout("%s\n", getAppDataEnv(getArg(0), getArg(1)));
		return;
	}
	if (hasArgs(1))
	{
		char *value = getAppDataEnv(nextArg(), NULL);

		errorCase_m(!value, "�w�肳�ꂽ�ϐ��͒�`����Ă��܂���B");
		cout("%s\n", value);
		return;
	}
	error_m("�s���ȃR�}���h����");
}
