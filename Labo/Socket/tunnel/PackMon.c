/*
	PackMon.exe /S

		/S ... 停止する。

	PackMon.exe [/NIC NIC-IP-ADDRESS] [/F OUTPUT-FILE]

		NIC-IP-ADDRESS ... ネットワークカードのIPアドレス
		OUTPUT-FILE ... 出力先ファイル, 省略時 == stdout

	★ファイアーウォールを無効にしないと受信パケットを取れないぽい。
*/

#include "C:\Factory\Common\Options\SockMonitor.h"
#include "C:\Factory\Common\Options\csvStream.h"

#define PROG_UUID "{5927b360-e75b-422d-a981-bbc244b7421d}"

static uint StopEventHdl;
static char *TargetNicIP;
static void *NextP;

// ---- 受信データ -> csvRow ----

static SYSTEMTIME *Time;
static uchar *RData;
static uint RSize;
static uint RIndex;
static uint RBitIndex;
static autoList_t *Row;

static uint NextBit(void)
{
	uint bit;

	if (RIndex < RSize)
	{
		bit = RData[RIndex] & 1 << 7 - RBitIndex;

		if (RBitIndex == 7)
		{
			RIndex++;
			RBitIndex = 0;
		}
		else
		{
			RBitIndex++;
		}
	}
	else
	{
		bit = 0;
	}
	return bit;
}
static uint NextValue(uint bitSize)
{
	uint value = 0;
	uint c;

	for (c = 0; c < bitSize; c++)
		if (NextBit())
			value |= 1 << bitSize - 1 - c;

	return value;
}

static void RecvDataParse(void)
{
	uint ipVersion = NextValue(4); // IP_VERSION
	uint c;

	addElement(Row, (uint)xcout("%u", ipVersion));

	if (ipVersion == 4)
	{
		uint ihl = NextValue(4); // IP_INTERNET_HEADER_LENGTH
		uint protocol;
		uchar ip[4];

		addElement(Row, (uint)xcout("%u", ihl));
		addElement(Row, (uint)xcout("%u", NextValue(6)));  // IP_DIFFERENTIATED_SERVICE_CODE_POINT
		addElement(Row, (uint)xcout("%u", NextValue(2)));  // IP_EXPLICIT_CONGESTION_NOTIFICATION
		addElement(Row, (uint)xcout("%u", NextValue(16))); // IP_TOTAL_LENGTH
		addElement(Row, (uint)xcout("%u", NextValue(16))); // IP_IDENTIFICATION
		addElement(Row, (uint)xcout("%u", NextValue(3)));  // IP_FLAGS
		addElement(Row, (uint)xcout("%u", NextValue(13))); // IP_FRAGMENT_OFFSET
		addElement(Row, (uint)xcout("%u", NextValue(8)));  // IP_TIME_TO_LIVE

		protocol = NextValue(8); // IP_PROTOCOL

		switch (protocol)
		{
		case 6:
			addElement(Row, (uint)strx("TCP"));
			break;

		case 17:
			addElement(Row, (uint)strx("UDP"));
			break;

		default:
			addElement(Row, (uint)xcout("%u", protocol));
			break;
		}

		addElement(Row, (uint)xcout("%u", NextValue(16))); // IP_HEADER_CHECKSUM

		ip[0] = NextValue(8); // IP_SOURCE_IP_ADDRESS_1
		ip[1] = NextValue(8); // IP_SOURCE_IP_ADDRESS_2
		ip[2] = NextValue(8); // IP_SOURCE_IP_ADDRESS_3
		ip[3] = NextValue(8); // IP_SOURCE_IP_ADDRESS_4

		addElement(Row, (uint)xcout("%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]));

		ip[0] = NextValue(8); // IP_DESTINATION_IP_ADDRESS_1
		ip[1] = NextValue(8); // IP_DESTINATION_IP_ADDRESS_2
		ip[2] = NextValue(8); // IP_DESTINATION_IP_ADDRESS_3
		ip[3] = NextValue(8); // IP_DESTINATION_IP_ADDRESS_4

		addElement(Row, (uint)xcout("%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]));

		// IP_OPTIONS
		{
			char *buff = strx("");

			for (c = 0; c + 5 < ihl; c++)
			{
				buff = addLine_x(buff, xcout("%08x", NextValue(32)));
			}
			addElement(Row, (uint)buff);
		}

		if (protocol == 6) // ? TCP
		{
			uint dataOffset;

			addElement(Row, (uint)xcout("%u", NextValue(16)));  // TCP_SOURCE_PORT
			addElement(Row, (uint)xcout("%u", NextValue(16)));  // TCP_DESTINATION_PORT
			addElement(Row, (uint)xcout("%u", NextValue(32)));  // TCP_SEQUENCE_NUMBER
			addElement(Row, (uint)xcout("%u", NextValue(32)));  // TCP_ACKNOWLEDGMENT_NUMBER

			dataOffset = NextValue(4); // TCP_DATA_OFFSET

			addElement(Row, (uint)xcout("%u", dataOffset));
			addElement(Row, (uint)xcout("%u", NextValue(3)));  // TCP_RESERVED
			addElement(Row, (uint)xcout("%u", NextValue(1)));  // TCP_NS
			addElement(Row, (uint)xcout("%u", NextValue(1)));  // TCP_CWR
			addElement(Row, (uint)xcout("%u", NextValue(1)));  // TCP_ECE
			addElement(Row, (uint)xcout("%u", NextValue(1)));  // TCP_URG
			addElement(Row, (uint)xcout("%u", NextValue(1)));  // TCP_ACK
			addElement(Row, (uint)xcout("%u", NextValue(1)));  // TCP_PSH
			addElement(Row, (uint)xcout("%u", NextValue(1)));  // TCP_RST
			addElement(Row, (uint)xcout("%u", NextValue(1)));  // TCP_SYN
			addElement(Row, (uint)xcout("%u", NextValue(1)));  // TCP_FIN
			addElement(Row, (uint)xcout("%u", NextValue(16)));  // TCP_WINDOW_SIZE
			addElement(Row, (uint)xcout("%u", NextValue(16)));  // TCP_CHECKSUM
			addElement(Row, (uint)xcout("%u", NextValue(16)));  // TCP_URGENT_POINTER

			// TCP_OPTIONS
			{
				char *buff = strx("");

				for (c = 0; c + 5 < dataOffset; c++)
				{
					buff = addLine_x(buff, xcout("%08x", NextValue(32)));
				}
				addElement(Row, (uint)buff);
			}
		}
		else if (protocol == 17) // ? UDP
		{
			addElement(Row, (uint)xcout("%u", NextValue(16)));  // UDP_SOURCE_PORT
			addElement(Row, (uint)xcout("%u", NextValue(16)));  // UDP_DESTINATION_PORT
			addElement(Row, (uint)xcout("%u", NextValue(16)));  // UDP_LENGTH
			addElement(Row, (uint)xcout("%u", NextValue(16)));  // UDP_CHECKSUM
		}
	}
	errorCase(RBitIndex); // ? 途中のビットで止まってる。

	while (getCount(Row) < 34)
		addElement(Row, (uint)strx(""));

	// DATA
	{
		char *buff = strx("");

		for (c = RIndex; c < RSize; c++)
		{
			buff = addLine_x(buff, xcout("%02x", RData[c]));
		}
		addElement(Row, (uint)buff);
	}
}
static void TimeParse(void)
{
#if 1
	// HACK: UTC
	addElement(Row, (uint)xcout(
		"%04u/%02u/%02u %02u:%02u:%02u.%03u"
		,Time->wYear
		,Time->wMonth
		,Time->wDay
		,Time->wHour
		,Time->wMinute
		,Time->wSecond
		,Time->wMilliseconds
		));
#else
	/*
		test
	*/
	addElement(Row, (uint)xcout("%u", Time->wYear));
	addElement(Row, (uint)xcout("%u", Time->wMonth));
	addElement(Row, (uint)xcout("%u", Time->wDayOfWeek));
	addElement(Row, (uint)xcout("%u", Time->wDay));
	addElement(Row, (uint)xcout("%u", Time->wHour));
	addElement(Row, (uint)xcout("%u", Time->wMinute));
	addElement(Row, (uint)xcout("%u", Time->wSecond));
	addElement(Row, (uint)xcout("%u", Time->wMilliseconds));
#endif
}
autoList_t *ParseRow(SYSTEMTIME *time, uchar *recvData, uint recvSize)
{
	Time = time;
	RData = recvData;
	RSize = recvSize;
	RIndex = 0;
	RBitIndex = 0;
	Row = newList();

	TimeParse();
	RecvDataParse();

	return Row;
}

// ----

static void RecvDataProc(uchar *recvData, uint recvSize)
{
	uchar *p = memAlloc(sizeof(void *) + sizeof(SYSTEMTIME) + sizeof(uint) + recvSize);

	*(void **)NextP = p;

	*(void **)p = NULL;
	NextP = (void *)p;
	p += sizeof(void *);

	GetSystemTime((SYSTEMTIME *)p);
	p += sizeof(SYSTEMTIME);

	*(uint *)p = recvSize;
	p += sizeof(uint);

	memcpy(p, recvData, recvSize);
}
static void SaveFile(void *record, char *file) // (file == NULL) -> stdout
{
	FILE *fp = file ? fileOpen(file, "wt") : stdout;

	cout("キャプチャデータを出力します...\n");

	while (record)
	{
		uchar *p = (uchar *)record;
		SYSTEMTIME *time;
		uchar *recvData;
		uint recvSize;

		record = *(void **)p;
		p += sizeof(void *);

		time = (SYSTEMTIME *)p;
		p += sizeof(SYSTEMTIME);

		recvSize = *(uint *)p;
		p += sizeof(uint);

		recvData = (uchar *)p;

		writeCSVRow_x(fp, ParseRow(time, recvData, recvSize));
	}
	if (file)
		fileClose(fp);

	cout("キャプチャデータを出力しました。\n");
}

static uint GetNicIndex(autoList_t *strNicIpList)
{
	uint nicIndex = 0; // 最初のNIC

	if (TargetNicIP)
	{
		char *strIp;
		uint index;

		foreach (strNicIpList, strIp, index)
			if (!strcmp(strIp, TargetNicIP))
				nicIndex = index;
	}
	cout("NIC -> %s\n", getLine(strNicIpList, nicIndex));
	return nicIndex;
}
static int IsKeep(void)
{
	if (handleWaitForMillis(StopEventHdl, 0)) // ? 停止イベント
	{
		cout("STOP_EVENT_OK\n");
		return 0;
	}
	return !hasKey() || getKey() != 0x1b;
}
int main(int argc, char **argv)
{
	char *file = NULL;
	void *top = NULL;

	StopEventHdl = eventOpen(PROG_UUID);

readArgs:
	if (argIs("/NIC"))
	{
		TargetNicIP = nextArg();
		goto readArgs;
	}
	if (argIs("/F"))
	{
		file = nextArg();
		goto readArgs;
	}

	if (argIs("/S"))
	{
		eventSet(StopEventHdl);
		goto endFunc;
	}

	if (file)
	{
		cout("出力ファイル: %s\n", file);
		createFile(file); // 出力テスト
	}

	cout("キャプチャを開始しました。\n");
	cout("停止するにはエスケープキーを押すか PackMon /S を実行して下さい。\n");

	NextP = &top;
	sockMonitor(GetNicIndex, RecvDataProc, IsKeep);
	SaveFile(top, file);

endFunc:
	handleClose(StopEventHdl);
}
