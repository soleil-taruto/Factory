#include "Bouyomichan.h"

static uint ToWord(uint pml, uint minval, uint maxval)
{
	if (pml == 0xffff)
		return 0xffff;

	errorCase(!m_isRange(pml, 0, 1000));

	return m_d2i((pml / 1000.0) * (maxval - minval) + minval);
}

static SockStream_t *SStrm;

static void SendWord(uint value)
{
	cout("SendWord: %u\n", value);

	SockSendChar(SStrm, value >> 0 & 0xff);
	SockSendChar(SStrm, value >> 8 & 0xff);
}
static void SendDblWord(uint value)
{
	cout("SendDblWord: %u\n", value);

	SockSendChar(SStrm, value >>  0 & 0xff);
	SockSendChar(SStrm, value >>  8 & 0xff);
	SockSendChar(SStrm, value >> 16 & 0xff);
	SockSendChar(SStrm, value >> 24 & 0xff);
}
static void SendString(char *str)
{
	SockSendToken(SStrm, str);
}

static uint Speed;
static uint Tone;
static uint Volume;
static uint Voice;
static char *Message;

static int Perform(int sock, uint prm_dummy)
{
	SStrm = CreateSockStream(sock, 20);

	SendWord(1); // COMMAND_READ_MESSAGE
	SendWord(Speed);
	SendWord(Tone);
	SendWord(Volume);
	SendWord(Voice);
	SendWord(2); // CHARSET_SJIS
	SendDblWord(strlen(Message));
	SendString(Message);

	SockFlush(SStrm);
	ReleaseSockStream(SStrm);
	SStrm = NULL;

	return 1;
}

/*
	domain    ... 棒読みちゃんのホスト名
	portno    ... 棒読みちゃんのポート番号
	message   ... 文字列
	speedPml  ... *PML
	tonePml   ... *PML
	volumePml ... *PML
	voice     ... 0 ～ 8

	*PML ... 0 ～ 1000, 0xffff == デフォルト
*/
void SendToBouyomichan(char *domain, uint portno, char *message, uint speedPml, uint tonePml, uint volumePml, uint voice)
{
	errorCase(m_isEmpty(domain));
	errorCase(!m_isRange(portno, 1, 0xffff));
	errorCase(!message);
	// speedPml
	// tonePml
	// volumePml
	errorCase(!m_isRange(voice, 0, 8));

	if (!*message)
		return;

	Speed  = ToWord(speedPml, 50, 300);
	Tone   = ToWord(tonePml,  50, 200);
	Volume = ToWord(volumePml, 0, 100);
	Voice = voice;
	Message = message;

	SClient(domain, portno, Perform, 0);
}
