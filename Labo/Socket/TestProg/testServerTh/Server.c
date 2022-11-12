#include "C:\Factory\Common\Options\SockServerTh.h"

static uint ConnectCounter;

static void PerformTh(int sock, char *ip)
{
	autoBlock_t *buffer = newBlock();
	char *line;

	ConnectCounter++;
	cout(">> %d (%u)\n", sock, ConnectCounter);

	while ((line = SockNextLine(buffer)) == NULL)
		if (SockRecvSequ(sock, buffer, 3000) == -1)
			break;

	if (!line)
		goto endfunc;

	reverseLine(line); // ‹t‚³‚É‚µ‚Ä•Ô‚·‚¾‚¯B

	setSize(buffer, 0);
	ab_addLine_x(buffer, line);

	while (getSize(buffer))
		if (SockSendSequ(sock, buffer, 3000) == -1)
			break;

	cout("<< %d\n", sock);

endfunc:
	releaseAutoBlock(buffer);
}

static int IdleTh(void)
{
	return !hasKey();
}
int main(int argc, char **argv)
{
	sockServerTh(
		PerformTh,
		hasArgs(1) ? toValue(nextArg()) : 59999, // port-no
		10, // connect-max
		IdleTh
		);
}
