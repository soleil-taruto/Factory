/*
	mpSend.exe <DOMAIN>:<PORTNO> <MESSAGE>
	mpSend.exe :<PORTNO> <MESSAGE>          == localhost:<PORTNO>
	mpSend.exe <DOMAIN> <MESSAGE>           == <DOMAIN>:80
	mpSend.exe "" <MESSAGE>                 == localhost:80
*/

#include "C:\Factory\Common\Options\SockClient.h"

static char *EncodeUrlMessage(char *message)
{
	autoBlock_t *lineBuff = newBlock();
	char *p;

	for (p = message; *p; p++)
	{
		if (*p == ' ')
		{
			addByte(lineBuff, '+');
		}
		else if (*p == '+' || *p == '%')
		{
			ab_addLine_x(lineBuff, xcout("%%%02x", *p));
		}
		else
		{
			addByte(lineBuff, *p);
		}
	}
	return ab_makeLine_x(lineBuff);
}

static char *MpMessage;

static void SendMpMessage(int sock)
{
	autoBlock_t *mq = newBlock();
	char *swrk;

	ab_addLine(mq, "q ?=");
	ab_addLine(mq, swrk = EncodeUrlMessage(MpMessage)); memFree(swrk);
	ab_addLine(mq, "\n");

	while (getSize(mq))
	{
		SockSendSequ(sock, mq, 3000);
	}
	releaseAutoBlock(mq);
}
int main(int argc, char **argv)
{
	uchar ip[4];
	char *domain;
	char *p;
	uint portno = 80;

	domain = nextArg();

	if (p = strchr(domain, ':'))
	{
		*p = '\0';
		portno = toValue(p + 1);
	}
	MpMessage = nextArg();

	*(uint *)ip = 0;
	sockClientUserTransmit(ip, domain, portno, SendMpMessage);
}
