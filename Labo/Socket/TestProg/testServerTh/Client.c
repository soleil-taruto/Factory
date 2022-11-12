#include "C:\Factory\Common\Options\SockClient.h"
#include "C:\Factory\DevTools\libs\RandData.h"

static int Connected;

static void Perform(int sock)
{
	autoBlock_t *buffer = newBlock();
	char *message;
	char *retMessage;

	cout("Connected\n");

	Connected = 1;
	message = MakeRandLineRange(0, 256 * 1024);

	ab_addLine(buffer, message);
	ab_addLine(buffer, "\r\n");

	while (getSize(buffer))
		if (SockSendSequ(sock, buffer, 3000) == -1)
			error();

	while (getSize(buffer) < strlen(message))
		if (SockRecvSequ(sock, buffer, 3000) == -1)
			error();

	retMessage = unbindBlock(buffer);
	reverseLine(retMessage);

	errorCase(strcmp(message, retMessage));

	memFree(message);
	memFree(retMessage);

	cout("OK!\n");
}

static void KeyWaitExit(void)
{
	cout("Press any key.\n");
	clearGetKey();
	exit(1);
}
int main(int argc, char **argv)
{
	uchar ip[4];

	addFinalizer(KeyWaitExit);

	if (argIs("/R"))
		mt19937_init32(toValue(nextArg()));
	else
		mt19937_init();

	*(uint *)ip = 0;

	if (hasArgs(2))
		sockClientUserTransmit(ip, getArg(0), toValue(getArg(1)), Perform);
	else
		sockClientUserTransmit(ip, "localhost", 59999, Perform);

	if (!Connected)
		error();

	sleep(500); // ˆêu‚Å•Â‚¶‚Ä‚µ‚Ü‚¤‚Ì‚ÅB
}
