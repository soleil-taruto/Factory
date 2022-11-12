#include "SClient.h"

// ---- IP ----

static autoList_t *ServerHosts;
static autoList_t *ServerIPs;
static uint LastIPIndex;

static uint GetServerIP(char *host)
{
	uint index = findLine(ServerHosts, host);

	if (index == getCount(ServerHosts))
	{
		if (100 < index) // overflow
		{
			LOGPOS();
			releaseDim(ServerHosts, 1);
			ServerHosts = newList();
			setCount(ServerIPs, 0);
		}
		addElement(ServerHosts, (uint)strx(host));
		addElement(ServerIPs, 0);
	}
	LastIPIndex = index;
	return getElement(ServerIPs, index);
}
static void SetServerIP(uint ip)
{
	setElement(ServerIPs, LastIPIndex, ip);
}

// ----

static int (*UserPerform)(int, uint);
static uint UserPrm;
static int Successful;

static void Perform(int sock)
{
	Successful = UserPerform(sock, UserPrm);
}
int SClient(char *serverHost, uint serverPort, int (*userPerform)(int, uint), uint userPrm) // ret: ? successful
{
	uchar ip[4];

	errorCase(m_isEmpty(serverHost));
	errorCase(!m_isRange(serverPort, 1, 65535));
	errorCase(!userPerform);
	// userPrm

	if (!ServerHosts) // init
	{
		ServerHosts = newList();
		ServerIPs = newList();
	}
	UserPerform = userPerform;
	UserPrm = userPrm;
	Successful = 0;

	*(uint *)ip = GetServerIP(serverHost);
	sockClientUserTransmit(ip, serverHost, serverPort, Perform);
	SetServerIP(*(uint *)ip);

	return Successful;
}
