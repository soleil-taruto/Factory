#include "Nectar.h"

#define COMMON_ID "{91ed4458-fe67-4093-a430-9dbf09db9904}" // shared_uuid@g

#define RECV_SIZE_MAX 20000000 // 20 MB

Nectar_t *CreateNectar(char *name)
{
	Nectar_t *i = (Nectar_t *)memAlloc(sizeof(Nectar_t));
	char *ident;

	sha512_localize();
	{
		sha512_makeHashLine(name);
		sha512_128_makeHexHash();

		ident = strx(sha512_hexHash);
	}
	sha512_unlocalize();

	i->EvData = eventOpen_x(xcout(COMMON_ID "_%s_Data", ident));
	i->EvCtrl = eventOpen_x(xcout(COMMON_ID "_%s_Ctrl", ident));
	i->EvSend = eventOpen_x(xcout(COMMON_ID "_%s_Send", ident));
	i->EvRecv = eventOpen_x(xcout(COMMON_ID "_%s_Recv", ident));

	return i;
}
void ReleaseNectar(Nectar_t *i)
{
	handleClose(i->EvData);
	handleClose(i->EvCtrl);
	handleClose(i->EvSend);
	handleClose(i->EvRecv);

	memFree(i);
}

// <-- cdtor

static int SendBit(Nectar_t *i, int data, int ctrl) // ret: ? 成功
{
	if (data)
		eventSet(i->EvData);

	if (ctrl)
		eventSet(i->EvCtrl);

	eventSet(i->EvSend);

	if (!handleWaitForMillis(i->EvRecv, 30000)) // 30 秒 -- 受信側が存在すること前提なので、長め。
	{
		cout("Warning: Nectar_送信タイムアウト\n");

		// 異常終了なのでクリア
		{
			handleWaitForMillis(i->EvData, 0);
			handleWaitForMillis(i->EvCtrl, 0);
			handleWaitForMillis(i->EvSend, 0);
			handleWaitForMillis(i->EvRecv, 100); // シビアなタイミングで受信された可能性を考慮して、受信側の _evRecv セットを少しだけ待つ。
		}
		return 0;
	}
	return 1;
}
static void Send(Nectar_t *i, autoBlock_t *message)
{
	uint index;
	uint bit;

	// 前回正常終了しなかった可能性を考慮して、念のためクリア
	{
		handleWaitForMillis(i->EvData, 0);
		handleWaitForMillis(i->EvCtrl, 0);
		handleWaitForMillis(i->EvSend, 0);
		handleWaitForMillis(i->EvRecv, 0);
	}
	SendBit(i, 0, 1);

	for (index = 0; index < getSize(message); index++)
	{
		int chr = getByte(message, index);

		for (bit = 1 << 7; bit; bit >>= 1)
			if (!SendBit(i, chr & bit, 0))
				return;
	}
	SendBit(i, 1, 1);
}
static int RecvBit(Nectar_t *i, uint *p_ret)
{
	if (!handleWaitForMillis(i->EvSend, 2000)) // 2 秒 -- タイムアウトしても送信中のメッセージは維持される。interrupt 確保のため、短め。
		return 0;

	*p_ret = 0;

	if (handleWaitForMillis(i->EvData, 0))
		*p_ret |= 1;

	if (handleWaitForMillis(i->EvCtrl, 0))
		*p_ret |= 2;

	eventSet(i->EvRecv);

	return 1;
}
static autoBlock_t *Recv(Nectar_t *i) // ret: NULL == 受信タイムアウト || サイズ超過
{
	autoBlock_t *retBuff = NULL;
	static autoBlock_t *buff;
	static uint chr;
	static uint bit;

	for (; ; )
	{
		uint ret;

		if (!RecvBit(i, &ret))
		{
			if (buff)
			{
				releaseAutoBlock(buff);
				buff = NULL;
			}
			break;
		}
		else if (ret == 2)
		{
			if (!buff)
				buff = newBlock();

			setSize(buff, 0);
			chr = 0;
			bit = 0;
		}
		else if (!buff)
		{
			// noop
		}
		else if (ret == 3)
		{
			retBuff = buff;
			buff = NULL;
			break;
		}
		else
		{
			chr <<= 1;
			chr |= ret;
			bit++;

			if (bit == 8)
			{
				if (RECV_SIZE_MAX <= getSize(buff))
				{
					releaseAutoBlock(buff);
					buff = NULL;
					break;
				}
				addByte(buff, chr);
				chr = 0;
				bit = 0;
			}
		}
	}
	return retBuff;
}

// ---- send ----

void NectarSend(Nectar_t *i, autoBlock_t *buff)
{
	Send(i, buff);
}
void NectarSendLine(Nectar_t *i, char *line)
{
	autoBlock_t gab;
	NectarSend(i, gndBlockLineVar(line, gab));
}

// ---- recv ----

/*
	クライアント応答用
*/
autoBlock_t *NectarRecv(Nectar_t *i)
{
	autoBlock_t *message;
	uint c;

	for (c = 0; c < 60; c += 2) // 60 秒 -- 相手側の処理時間 + 応答が必ずあることが前提なので、長め。
	{
		message = Recv(i);

		if (message)
			break;
	}
	return message;
}

/*
	サーバー用
	受信を待機する場合、時間を空けずに繰り返し呼び出すこと。
*/
autoBlock_t *NectarReceipt(Nectar_t *i)
{
	return Recv(i);
}

// ----
