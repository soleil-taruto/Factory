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

		errorCase_m(!value, "指定された変数は定義されていません。");
		cout("%s\n", value);
		return;
	}
	error_m("不正なコマンド引数");
}
