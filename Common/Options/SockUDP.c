#include "SockUDP.h"

#define RECV_BUFF_SIZE (1024 * 1024 * 2)

static uchar *GetDefIP(char *domain)
{
	static uchar ip[4];

	memset(ip, 0x00, 4);
	return ip;
}
int sockUDPOpenSend(void)
{
	int sock;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	errorCase(sock == -1);
	SockPostOpen(sock);

	return sock;
}
int sockUDPOpenRecv(uint portno)
{
	int retval;
	int sock;
	struct sockaddr_in sa;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	errorCase(sock == -1);
	SockPostOpen(sock);

	memset(&sa, 0x00, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	sa.sin_port = htons((unsigned short)portno);

	retval = bind(sock, (struct sockaddr *)&sa, sizeof(sa));
	errorCase(retval != 0); // ? == -1

	return sock;
}
void sockUDPSendBlock(int sock, uchar ip[4], char *domain, uint portno, uchar *data, uint dataSize)
{
	char *strip;
	struct sockaddr_in sa;
	int retval;

	if (!dataSize) // ? 空データ -> 送信しない。
		return;

	if (!ip)
		ip = GetDefIP(domain);

	if (!*(uint *)ip) // ? 0.0.0.0
	{
		errorCase(!domain);
		sockLookup(ip, domain);

		if (!*(uint *)ip)
		{
			cout("Warning: UDP send ip == 0.0.0.0\n");
			return;
		}
	}
	strip = SockIp2Line(ip);

	memset(&sa, 0x00, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = inet_addr(strip);
	sa.sin_port = htons((unsigned short)portno);

	retval = sendto(sock, data, dataSize, 0, (struct sockaddr *)&sa, sizeof(sa));

	if (retval != dataSize)
		cout("Warning: UDP sendto() %u -> %u\n", dataSize, retval);
}
void sockUDPSend(int sock, uchar ip[4], char *domain, uint portno, autoBlock_t *block)
{
	sockUDPSendBlock(sock, ip, domain, portno, directGetBuffer(block), getSize(block));
}
uint sockUDPRecvBlock(int sock, uint millis, uchar *buff, uint buffSize)
{
	int retval;

	retval = SockTransmit(sock, buff, buffSize, millis, 0);

	if (retval == -1) // ? 空データ（空のUDPパケット？）を受信したとき -1 になるっぽい。
		return 0;

	errorCase(!m_isRange(retval, 0, buffSize));
	return retval;
}
autoBlock_t *sockUDPRecv(int sock, uint millis)
{
	static uchar *buff;
	uint recvSize;

	if (!buff)
		buff = memAlloc(RECV_BUFF_SIZE);

	recvSize = sockUDPRecvBlock(sock, millis, buff, RECV_BUFF_SIZE);

	if (recvSize)
		return recreateBlock(buff, recvSize);

	return NULL;
}
void sockUDPClose(int sock)
{
	int retval;

	SockPreClose(sock);
	retval = closesocket(sock);
	errorCase(retval);
}
