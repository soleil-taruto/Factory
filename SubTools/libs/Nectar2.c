#include "Nectar2.h"

#define COMMON_ID "{8cf92c5e-c4f7-4867-9e1a-5371bb53aa63}" // shared_uuid@g

enum
{
	E_SEND,
	E_RECV,
	E_BIT_0,
	E_BIT_1,
	E_BIT_2,
	E_BIT_3,
	E_BIT_4,
	E_BIT_5,
	E_BIT_6,
	E_BIT_7,

	E_MAX, // == NECTAR2_E_MAX
};

// cdtor >

Nectar2_t *CreateNectar2(char *ident) // ident: 名前付きイベント名に使うので注意！
{
	Nectar2_t *i = (Nectar2_t *)memAlloc(sizeof(Nectar2_t));
	uint index;

	for (index = 0; index < E_MAX; index++)
	{
		i->Handles[index] = eventOpen_x(xcout("Nectar2_" COMMON_ID "_%s_%u", ident, index));
	}
	i->RecvBuff = NULL;
	return i;
}
void ReleaseNectar2(Nectar2_t *i)
{
	uint index;

	for (index = 0; index < E_MAX; index++)
	{
		handleClose(i->Handles[index]);
	}
	if (i->RecvBuff)
		releaseAutoBlock(i->RecvBuff);

	memFree(i);
}

// < cdtor

static void Set(Nectar2_t *i, uint index)
{
	eventSet(i->Handles[index]);
}
static int Get(Nectar2_t *i, uint index, uint millis)
{
	return handleWaitForMillis(i->Handles[index], millis);
}

// ---- send ----

static int SendChar(Nectar2_t *i, int chr) // ret: ? 成功
{
	uint bit;

	for (bit = 0; bit < 8; bit++)
		if (chr & 1 << bit)
			Set(i, E_BIT_0 + bit);

	Get(i, E_RECV, 0); // clear
	Set(i, E_SEND);

	return Get(i, E_RECV, 15000); // 送信タイムアウト
}
void Nectar2Send(Nectar2_t *i, autoBlock_t *message)
{
	uint index;

	for (index = 0; index < getSize(message); index++)
	{
		if (!SendChar(i, getByte(message, index)))
		{
			cout("Warning: 送信失敗 @ Nectar2\n");
			break;
		}
	}
}
void Nectar2SendLine(Nectar2_t *i, char *line)
{
	autoBlock_t gab;
	Nectar2Send(i, gndBlockLineVar(line, gab));
}
void Nectar2SendLine_x(Nectar2_t *i, char *line)
{
	Nectar2SendLine(i, line);
	memFree(line);
}
void Nectar2SendChar(Nectar2_t *i, int chr)
{
	static autoBlock_t *message;

	if (!message)
	{
		message = newBlock();
		addByte(message, 0x00); // dummy
	}
	setByte(message, 0, chr);

	Nectar2Send(i, message);
}

// ---- recv ----

static int RecvChar(Nectar2_t *i) // ret: -1 == 失敗 || タイムアウト
{
	uint bit;
	int chr = 0;

	if (!Get(i, E_SEND, 2000)) // 受信タイムアウト
		return -1;

	for (bit = 0; bit < 8; bit++)
		if (Get(i, E_BIT_0 + bit, 0))
			chr |= 1 << bit;

	Set(i, E_RECV);
	return chr;
}
autoBlock_t *Nectar2Recv(Nectar2_t *i, int delimiter)
{
	autoBlock_t *ret = NULL;

	if (!i->RecvBuff)
		i->RecvBuff = newBlock();

	for (; ; )
	{
		int chr = RecvChar(i);

		if (chr == -1)
			break;

		if (chr == delimiter)
		{
			ret = i->RecvBuff;
			i->RecvBuff = NULL;
			break;
		}
		addByte(i->RecvBuff, chr);
	}
	return ret;
}
char *Nectar2RecvLine(Nectar2_t *i, int delimiter)
{
	autoBlock_t *message = Nectar2Recv(i, delimiter);

	if (!message)
		return NULL;

	return unbindBlock2Line(message);
}

// ----
