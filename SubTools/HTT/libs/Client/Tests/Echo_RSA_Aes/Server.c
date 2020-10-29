/*
	Service.dat Ç…à»â∫Çìoò^Ç∑ÇÈÇ±Ç∆ÅB

		EchoTest_RSA_Aes C:\Factory\SubTools\HTT\libs\Client\Test\Echo_RSA_Aes\Server.exe
*/

#include "C:\Factory\SubTools\HTT\libs\CRPC_RSA_Aes.h"

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
