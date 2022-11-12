#include "SockMonitor.h"

#define SIO_RCVALL 0x98000001 // == _WSAIOW(IOC_VENDOR,1)

#define RECVBUFFSIZE (128 * 1024)
#define RECVWAITMILLIS 2000

void sockMonitor(
	uint (*selectNicProc)(autoList_t *strNicIpList),
	void (*recvedDataProc)(uchar *recvedData, uint recvedSize),
	int (*idleProc)(void)
	)
{
	uchar *recvBuff = memAlloc(RECVBUFFSIZE);
	char *strNicIp;
	int sock;
	struct sockaddr_in sa;
	int retval;
	uint waitMillis = 0;

	errorCase(!selectNicProc);
	errorCase(!recvedDataProc);
	errorCase(!idleProc);

	SockStartup();

	{
		autoList_t *nicIpList = sockLookupList("");
		autoList_t *strNicIpList = newList();
		uint nicIp;
		uint index;

		foreach (nicIpList, nicIp, index)
			addElement(strNicIpList, (uint)strx(SockIp2Line((uchar *)&nicIp)));

		index = selectNicProc(strNicIpList);

		if (index < getCount(strNicIpList))
			strNicIp = strx(getLine(strNicIpList, index)); // 選択されたIP -> strNicIp
		else
			strNicIp = NULL; // キャンセルされた。

		releaseAutoList(nicIpList);
		releaseDim(strNicIpList, 1);

		if (!strNicIp)
			goto l_cleanup;
	}

	sock = socket(AF_INET, SOCK_RAW, IPPROTO_IP);
	errorCase(sock == -1);
	SockPostOpen(sock);

	memset(&sa, 0x00, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = inet_addr(strNicIp); // htonl(INADDR_ANY); はダメみたい。
	sa.sin_port = htons(0);

	retval = bind(sock, (struct sockaddr *)&sa, sizeof(sa));
	errorCase(retval != 0); // ? == -1

	/*
	// 非ブロッキングモード
	{
		unsigned long arg = 1L;
		errorCase(ioctlsocket(sock, FIONBIO, &arg) != 0);
	}
	*/

	// プロミスキャスモード
	{
		unsigned long optval = 1; // PROMISC
		errorCase(ioctlsocket(sock, SIO_RCVALL, &optval) != 0);
	}

	while (idleProc())
	{
		retval = SockWait(sock, RECVWAITMILLIS, 0);
		errorCase(retval == -1);

		if (retval)
		{
			retval = recv(sock, recvBuff, RECVBUFFSIZE, 0);
			errorCase(retval < 1 || RECVBUFFSIZE < retval);
			recvedDataProc(recvBuff, retval);
		}
	}

	SockPreClose(sock);
	retval = closesocket(sock);
	errorCase(retval);

	memFree(strNicIp);

l_cleanup:
	SockCleanup();

	memFree(recvBuff);
}
