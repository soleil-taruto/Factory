#include "BoomTools.h"

uint boom_GetCRC16(autoBlock_t *credential, int flag, autoBlock_t *recvOrSendData)
{
	uint crc16 = crc16Start();

	crc16 = crc16UpdateBlock(crc16, directGetBuffer(credential), getSize(credential));
	crc16 = crc16Update(crc16, flag);
	crc16 = crc16UpdateBlock(crc16, directGetBuffer(recvOrSendData), getSize(recvOrSendData));

	return crc16Finish(crc16);
}
