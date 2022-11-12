#include "C:\Factory\Common\Options\SockMonitor.h"

/*
static uint SelectNicIp(autoList_t *strNicIpList)
{
	return 0; // �ŏ���NIC
}
*/
static uint SelectNicIp(autoList_t *strNicIpList)
{
	if (getCount(strNicIpList) == 0)
	{
		cout("NIC�����o�ł��܂���ł����B\n");
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
			cout("�L�����Z�����܂����B\n");
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

	cout("����M�f�[�^(%u)\n", dataSize);

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
			cout("�I�����܂��B\n");
			retval = 0;
		}
	}
	return retval;
}
int main(int argc, char **argv)
{
	cout("�J�n���܂��B\n");
	sockMonitor(SelectNicIp, RecvedDataProc, IdleProc);
	cout("�I�����܂����B\n");
}
