#include "C:\Factory\Common\Options\SockMonitor.h"

static uchar *RData;
static uint RSize;
static uint RIndex;
static uint RBitIndex;

#define IsReadEnd() \
	(RIndex + 1 == RSize && RBitIndex == 8)

#define IsReadPostEnd() \
	(RIndex == RSize)

static uint NextBit(void)
{
	uint bit;

	if (RBitIndex == 8)
	{
		RIndex++;
		RBitIndex = 0;
	}
	if (RIndex < RSize)
	{
		bit = RData[RIndex] & 1 << 7 - RBitIndex;
		RBitIndex++;
	}
	else
	{
		bit = 0;
	}
	return bit;
}
static uint NextValue(uint bitSize, char *name) // bitSize: 1-
{
	uint value = 0;
	uint c;

	for (c = 0; c < bitSize; c++)
		if (NextBit())
			value |= 1 << bitSize - 1 - c;

	cout("%s[%u]: ", name, bitSize);

	{
		char *fmt = xcout("%%0%ux", (bitSize + 3) / 4);
		cout(fmt, value);
		memFree(fmt);
	}
	cout(" (%u)", value);

	if (IsReadPostEnd())
		cout(" !");

	cout("\n");
	return value;
}

static void RecvDataProc(uchar *recvData, uint recvSize)
{
	uint c;

	// RInit
	{
		RData = recvData;
		RSize = recvSize;
		RIndex = 0;
		RBitIndex = 0;
	}
	cout("#\n");

	if (NextValue(4, "IP_VERSION") == 4)
	{
		uint ihl = NextValue(4, "IP_INTERNET_HEADER_LENGTH");
		uint protocol;

		NextValue(6, "IP_DIFFERENTIATED_SERVICE_CODE_POINT");
		NextValue(2, "IP_EXPLICIT_CONGESTION_NOTIFICATION");
		NextValue(16, "IP_TOTAL_LENGTH");
		NextValue(16, "IP_IDENTIFICATION");
		NextValue(3, "IP_FLAGS");
		NextValue(13, "IP_FRAGMENT_OFFSET");
		NextValue(8, "IP_TIME_TO_LIVE");
		protocol = NextValue(8, "IP_PROTOCOL");
		NextValue(16, "IP_HEADER_CHECKSUM");
		NextValue(8, "IP_SOURCE_IP_ADDRESS_1");
		NextValue(8, "IP_SOURCE_IP_ADDRESS_2");
		NextValue(8, "IP_SOURCE_IP_ADDRESS_3");
		NextValue(8, "IP_SOURCE_IP_ADDRESS_4");
		NextValue(8, "IP_DESTINATION_IP_ADDRESS_1");
		NextValue(8, "IP_DESTINATION_IP_ADDRESS_2");
		NextValue(8, "IP_DESTINATION_IP_ADDRESS_3");
		NextValue(8, "IP_DESTINATION_IP_ADDRESS_4");

		for (c = 0; c + 5 < ihl; c++)
		{
			char *name = xcout("IP_OPTIONS(%u)", c);
			NextValue(32, name);
			memFree(name);
		}
		if (protocol == 6) // ? TCP
		{
			uint dataOffset;

			NextValue(16, "TCP_SOURCE_PORT");
			NextValue(16, "TCP_DESTINATION_PORT");
			NextValue(32, "TCP_SEQUENCE_NUMBER");
			NextValue(32, "TCP_ACKNOWLEDGMENT_NUMBER");
			dataOffset = NextValue(4, "TCP_DATA_OFFSET");
			NextValue(3, "TCP_RESERVED");
			NextValue(1, "TCP_NS");
			NextValue(1, "TCP_CWR");
			NextValue(1, "TCP_ECE");
			NextValue(1, "TCP_URG");
			NextValue(1, "TCP_ACK");
			NextValue(1, "TCP_PSH");
			NextValue(1, "TCP_RST");
			NextValue(1, "TCP_SYN");
			NextValue(1, "TCP_FIN");
			NextValue(16, "TCP_WINDOW_SIZE");
			NextValue(16, "TCP_CHECKSUM");
			NextValue(16, "TCP_URGENT_POINTER");

			for (c = 0; c + 5 < dataOffset; c++)
			{
				char *name = xcout("TCP_OPTIONS(%u)", c);
				NextValue(32, name);
				memFree(name);
			}
		}
		else if (protocol == 17) // ? UDP
		{
			NextValue(16, "UDP_SOURCE_PORT");
			NextValue(16, "UDP_DESTINATION_PORT");
			NextValue(16, "UDP_LENGTH");
			NextValue(16, "UDP_CHECKSUM");
		}
	}
	if (m_isRange(RBitIndex, 1, 7)) // ? “r’†‚Ìƒrƒbƒg‚ÅŽ~‚Ü‚Á‚Ä‚éB
		NextValue(8 - RBitIndex, "ODD_DATA");

	if (RBitIndex == 8)
		RIndex++;

	cout("[DATA]\n");

	for (c = RIndex; c < RSize; c++)
	{
		cout("%02x", RData[c]);
	}
	cout("\n");
	cout("[DATA_ASCII]\n");

	{
		autoBlock_t gab;
		char *buff;
		buff = toPrintLine(gndBlockVar(RData + RIndex, RSize - RIndex, gab), 1);
		cout("%s\n", buff);
		memFree(buff);
	}
}

static uint GetNicIndex(autoList_t *strNicIpList)
{
	return 0; // Å‰‚ÌNIC
}
static int IsKeep(void)
{
	return !hasKey() || getKey() != 0x1b;
}
int main(int argc, char **argv)
{
	sockMonitor(GetNicIndex, RecvDataProc, IsKeep);
}
