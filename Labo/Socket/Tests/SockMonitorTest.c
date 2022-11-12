#include "C:\Factory\Common\Options\SockMonitor.h"

/*
static uint SelectNicIp(autoList_t *strNicIpList)
{
	return 0; // 最初のNIC
}
*/
static uint SelectNicIp(autoList_t *strNicIpList)
{
	if (getCount(strNicIpList) == 0)
	{
		cout("NICを検出できませんでした。\n");
		return UINTMAX;
	}
	if (getCount(strNicIpList) == 1)
	{
		return 0;
	}

	{
		char *line = selectLine(strNicIpList);
		uint index;

		if (!line)
		{
			cout("キャンセルしました。\n");
			return UINTMAX;
		}
		index = findLine(strNicIpList, line);

		errorCase(getCount(strNicIpList) <= index); // ? not found

		memFree(line);
		return index;
	}
}
static void RecvedDataProc(uchar *data, uint dataSize)
{
	autoBlock_t gab;
	char *line;

	cout("■受信データ(%u)\n", dataSize);

	line = toPrintLine(gndBlockVar(data, dataSize, gab), 1);
	cout("%s\n", line);
	memFree(line);
}
static int IdleProc(void)
{
	int retval = 1;

	while (hasKey())
	{
		if (getKey() == 0x1b)
		{
			cout("終了します。\n");
			retval = 0;
		}
	}
	return retval;
}
int main(int argc, char **argv)
{
	cout("開始します。\n");
	sockMonitor(SelectNicIp, RecvedDataProc, IdleProc);
	cout("終了しました。\n");
}
