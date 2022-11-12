/*
	‚±‚¿‚ç‚Í‹Œƒo[ƒWƒ‡ƒ“
	Œ®‚ª‚P‚Â‚Ìê‡A“¯‚¶Œ®‚Å‚Q‰ñŠ|‚¯‚éB
*/

#include "RingCipher.h"

#define BLOCK_SIZE 16

void rcphrEncryptor(autoBlock_t *block, autoList_t *keyTableList, int doEncrypt)
{
	uint index;

	errorCase(!block);
	errorCase(!keyTableList);

	errorCase(getSize(block) < BLOCK_SIZE * 2);
	errorCase(getSize(block) % BLOCK_SIZE != 0);
	errorCase(getCount(keyTableList) == 0);

	if (getCount(keyTableList) == 1)
	{
		for (index = 0; index < 2; index++)
		{
			camellia_cbcRing(
				(camellia_keyTable_t *)getElement(keyTableList, 0),
				block,
				block,
				doEncrypt
				);
		}
	}
	else
	{
		for (index = 0; index < getCount(keyTableList); index++)
		{
			camellia_cbcRing(
				(camellia_keyTable_t *)getElement(keyTableList, doEncrypt ? index : getCount(keyTableList) - 1 - index),
				block,
				block,
				doEncrypt
				);
		}
	}
}

static void SwapCounter2(autoBlock_t *block)
{
	autoBlock_t *farBlock;
	autoBlock_t *nearBlock;

	farBlock  = unaddBytes(block, BLOCK_SIZE);
	nearBlock = unaddBytes(block, BLOCK_SIZE);

	ab_addBytes_x(block, farBlock);
	ab_addBytes_x(block, nearBlock);
}
void rcphrEncryptorBlock(autoBlock_t *block, autoList_t *keyTableList, uint64 counter2[2])
{
	errorCase(!block);
//	errorCase(!keyTableList); // ‰º‚ÌŠÖ”‚É“n‚·B
	errorCase(!counter2);

	cphrAddPadding(block);

	ab_addValue(block, (uint)(counter2[0] >>  0));
	ab_addValue(block, (uint)(counter2[0] >> 32));
	ab_addValue(block, (uint)(counter2[1] >>  0));
	ab_addValue(block, (uint)(counter2[1] >> 32));

	cphrAddHash(block);
	SwapCounter2(block);
	rcphrEncryptor(block, keyTableList, 1);
}
int rcphrDecryptorBlock(autoBlock_t *block, autoList_t *keyTableList, uint64 counter2[2]) // ret: 0 == block ‚Í”j‘¹‚µ‚Ä‚¢‚éB
{
	errorCase(!block);
//	errorCase(!keyTableList); // ‰º‚ÌŠÖ”‚É“n‚·B
	errorCase(!counter2);

	if (getSize(block) < BLOCK_SIZE * 2)
		return 0;

	if (getSize(block) % BLOCK_SIZE != 0)
		return 0;

	rcphrEncryptor(block, keyTableList, 0);
	SwapCounter2(block);

	if (!cphrUnaddHash(block))
		return 0;

	counter2[1]  = (uint64)ab_unaddValue(block) << 32;
	counter2[1] |= (uint64)ab_unaddValue(block) <<  0;
	counter2[0]  = (uint64)ab_unaddValue(block) << 32;
	counter2[0] |= (uint64)ab_unaddValue(block) <<  0;

	return cphrUnaddPadding(block);
}
