#include "Mutector.h"

#define COMMON_ID "{fab3c841-8891-4273-8bd1-50525845fea7}" // shared_uuid@g

#define RECV_SIZE_MAX 20000000 // 20 MB

enum
{
	M_Sender,
	M_Recver,
	M_Sync_0,
	M_Sync_1,
	M_Sync_2,
	M_Bit_0_0,
	M_Bit_0_1,
	M_Bit_0_2,
	M_Bit_1_0,
	M_Bit_1_1,
	M_Bit_1_2,

	M_MAX, // == MUTECTOR_MTX_MAX
};

Mutector_t *CreateMutector(char *name)
{
	Mutector_t *i = (Mutector_t *)memAlloc(sizeof(Mutector_t));
	char *ident;
	uint index;

	sha512_localize();
	{
		sha512_makeHashLine(name);
		sha512_128_makeHexHash();

		ident = strx(sha512_hexHash);
	}
	sha512_unlocalize();

	for (index = 0; index < MUTECTOR_MTX_MAX; index++)
	{
		i->Mtxs[index] = mutexOpen_x(xcout(COMMON_ID "_%s_%u", ident, index));
		i->Statuses[index] = 0;
	}
	memFree(ident);

	return i;
}
void ReleaseMutector(Mutector_t *i)
{
	uint index;

	for (index = 0; index < MUTECTOR_MTX_MAX; index++)
	{
		if (i->Statuses[index])
			mutexRelease(i->Mtxs[index]);

		handleClose(i->Mtxs[index]);
	}
	memFree(i);
}

// <-- cdtor

static void Set(Mutector_t *i, uint index, int status)
{
	if (i->Statuses[index] ? !status : status)
	{
		if (status)
			handleWaitForever(i->Mtxs[index]);
		else
			mutexRelease(i->Mtxs[index]);

		i->Statuses[index] = status;
	}
}
static void Clear(Mutector_t *i, int status)
{
	uint index;

	for (index = MUTECTOR_MTX_MAX; index; index--) // 排他用を最後に解放したい。
	{
		Set(i, index - 1, status);
	}
}
static int TrySet(Mutector_t *i, uint index)
{
	errorCase(i->Statuses[index]); // この系は無いはず！

	if (handleWaitForMillis(i->Mtxs[index], 0))
	{
		i->Statuses[index] = 1;
		return 1;
	}
	return 0;
}
static int Get(Mutector_t *i, uint index)
{
	errorCase(i->Statuses[index]); // この系は無いはず！

	if (handleWaitForMillis(i->Mtxs[index], 0))
	{
		mutexRelease(i->Mtxs[index]);
		return 0;
	}
	return 1;
}

// ---- send ----

static uint M0 = 0;
static uint M1 = 1;

static void SendBit(Mutector_t *i, int b0, int b1)
{
	M0++;
	M1++;
	M0 %= 3;
	M1 %= 3;

	Set(i, M_Sync_0 + M1, 1);
	Set(i, M_Sync_0 + M0, 0);
	Set(i, M_Bit_0_0 + M1, b0);
	Set(i, M_Bit_1_0 + M1, b1);
}
void MutectorSend(Mutector_t *i, autoBlock_t *message)
{
	uint index;
	uint bit;

	Set(i, M_Sender, 1); // 送信側の排他

	// Recver.Perform() 実行中かどうか検査
	{
		if (
			TrySet(i, M_Sync_0) &&
			TrySet(i, M_Sync_1) &&
			TrySet(i, M_Sync_2)
			)
			goto endFunc; // recver is not running

		Set(i, M_Sync_0, 0);
		Set(i, M_Sync_1, 0);
		Set(i, M_Sync_2, 0);
	}

	SendBit(i, 1, 1);

	for (index = 0; index < getSize(message); index++)
	{
		int chr = getByte(message, index);

		for (bit = 1 << 7; bit; bit >>= 1)
		{
			if (chr & bit)
				SendBit(i, 0, 1);
			else
				SendBit(i, 1, 0);
		}
	}
	SendBit(i, 0, 0);
	SendBit(i, 0, 0);
	SendBit(i, 0, 0);

endFunc:
	Clear(i, 0);
}
void MutectorSendLine(Mutector_t *i, char *line)
{
	autoBlock_t gab;
	MutectorSend(i, gndBlockLineVar(line, gab));
}

// ---- recv_サーバー ----

static Mutector_t *R_I;
static int (*R_Interlude)(void);
static void (*R_Recved)(autoBlock_t *);

static uint R_Elapsed;

static autoBlock_t *R_Buff;
static int R_Chr;
static int R_Bit;

static void RecvedBit(uint bit)
{
	R_Chr <<= 1;
	R_Chr |= bit;
	R_Bit++;

	if (R_Bit == 8)
	{
		if (RECV_SIZE_MAX <= getSize(R_Buff))
		{
			cout("Warning: 受信サイズ超過\n");

			releaseAutoBlock(R_Buff);
			R_Buff = NULL;
//			R_Chr = 0;
//			R_Bit = 0;
		}
		else
		{
			addByte(R_Buff, R_Chr);
			R_Chr = 0;
			R_Bit = 0;
		}
	}
}
static void RecvBit(uint m0, uint m1, uint m2)
{
	Set(R_I, M_Sync_0 + m1, 1);

	// idling
	{
		static uint millis;

		if (!Get(R_I, M_Sync_0 + m2))
		{
			if (millis < 200)
				millis++;

			sleep(millis);
		}
		else
		{
			millis = 0;
		}

		R_Elapsed += millis + 1;
	}

	Set(R_I, M_Sync_0 + m0, 0);

	{
		int b0 = Get(R_I, M_Bit_0_0 + m1);
		int b1 = Get(R_I, M_Bit_1_0 + m1);

		if (b0 && b1)
		{
			if (!R_Buff)
				R_Buff = newBlock();

			setSize(R_Buff, 0);
			R_Chr = 0;
			R_Bit = 0;
		}
		else if (!R_Buff)
		{
			// noop
		}
		else if (b0)
		{
			RecvedBit(0);
		}
		else if (b1)
		{
			RecvedBit(1);
		}
		else
		{
			R_Recved(R_Buff);

			releaseAutoBlock(R_Buff);
			R_Buff = NULL;
//			R_Chr = 0;
//			R_Bit = 0;
		}
	}
}
static void Recv(void)
{
	if (!TrySet(R_I, M_Recver))
	{
		cout("Warning: already Perform() running");
		return;
	}
	Set(R_I, M_Sync_0, 1);

	for (; ; )
	{
		RecvBit(0, 1, 2);
		RecvBit(1, 2, 0);
		RecvBit(2, 0, 1);

		if (2000 <= R_Elapsed)
		{
			R_Elapsed -= 2000;

			if (!R_Interlude())
				break;
		}
	}
	Clear(R_I, 0);
}
void MutectorRecv(Mutector_t *i, int (*interlude)(void), void (*recved)(autoBlock_t *))
{
	R_I = i;
	R_Interlude = interlude;
	R_Recved = recved;

	Recv();
}

// ----
