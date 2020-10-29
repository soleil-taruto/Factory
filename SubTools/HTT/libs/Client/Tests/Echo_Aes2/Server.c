/*
	Service.dat Ç…à»â∫Çìoò^Ç∑ÇÈÇ±Ç∆ÅB

		EchoTest_Aes2 C:\Factory\SubTools\HTT\libs\Client\Test\Echo_Aes2\Server.exe
*/

#include "C:\Factory\SubTools\HTT\libs\CRPC_Aes2.h"

autoBlock_t *CRPC_GetRawKey(void)
{
	return makeBlockHexLine(
		"aca545f2e563d2d8aca14f994845cc6052c8ec8c577873badd9399e1f01bf366"
		"9d3743295d85dc92650356ffab2822f350c1e608aef336ae29613feae25ccb0b"
		);
		// .\\Client.c Ç∆çáÇÌÇπÇÈÇ±Ç∆ÅB
}
void CRPC_Init(void)
{
	// noop
}
autoBlock_t *ServiceCRPC(autoBlock_t *recvData, uint phase)
{
	autoBlock_t *sendData = newBlock();

	ab_addLine(sendData, "You sent [");
	addBytes(sendData, recvData);
	ab_addLine(sendData, "]");

	RPC_SendCompleteAndDisconnect();
	return sendData;
}
