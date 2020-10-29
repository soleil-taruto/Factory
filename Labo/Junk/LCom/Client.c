#include "C:\Factory\Common\all.h"
#include "LCom.h"

int main(int argc, char **argv)
{
	char *resLine = LComSendLine(nextArg(), 1);

	cout("%s\n", resLine);
	memFree(resLine);
}
