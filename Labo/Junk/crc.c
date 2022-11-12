#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\crc.h"

int main(int argc, char **argv)
{
	if (argIs("/L"))
	{
		char *line = nextArg();
		cout("crc8-16-32: %02x %04x %08x\n", crc8CheckLine(line), crc16CheckLine(line), crc32CheckLine(line));
		return;
	}
	if (argIs("/F"))
	{
		char *file = nextArg();
		cout("crc8-16-32: %02x %04x %08x\n", crc8CheckFile(file), crc16CheckFile(file), crc32CheckFile(file));
		return;
	}
}
