/*
	uuid.exe [/U] [/P]

		/U ... 大文字にする。
		/P ... 表示のみ。(エディタを開かない)
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\CRandom.h"

int main(int argc, char **argv)
{
	char *uuid = MakeUUID(1);

	if (argIs("/U"))
		toUpperLine(uuid);

	cout("%s\n", uuid);

	if (!argIs("/P")) // ? not Print only
		viewLine(uuid);

	memFree(uuid);
}
