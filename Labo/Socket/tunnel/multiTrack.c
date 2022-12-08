/*
	multiTrack.exe RECV-PORT DEF-FWD-HOST DEF-FWD-PORT /S

		/S ... ��~����B

	multiTrack.exe RECV-PORT DEF-FWD-HOST DEF-FWD-PORT [/C CONNECT-MAX] [TRACK-NAME FWD-HOST FWD-PORT | /F FWD-FILE]...

		CONNECT-MAX ... �ő�ڑ���, �ȗ����� 1000
		TRACK-NAME ... �g���b�N��
		FWD-HOST ... �]����z�X�g��
		FWD-PORT ... �]����|�[�g�ԍ�
		FWD-FILE ... �]���惊�X�g��`�t�@�C��

		�� DEF-FWD-HOST �� NONE ���w�肷��ƁA�f�t�H���g�̓]����͖����ɂȂ�܂��B

		�� DEF-FWD-HOST �� RED ���w�肷��ƁA���_�C���N�g�E���[�h�i�g���b�N�����]����j�ɂȂ�܂��B

	- - - -

	�]���惊�X�g��`�t�@�C��

		�e�L�X�g�t�@�C���ŁA��s�Ɣ��p�Z�~�R�����Ŏn�܂�s�͖������܂��B
		�e�s�͈ȉ��̂悤�ɋL�q���܂��B

			�z�X�g�� + �� + �]����z�X�g�� + �� + �]����|�[�g�ԍ�
*/

#include "libs\Tunnel.h"

#define TRACKNAME_LENMAX 100
#define HOST_LENMAX 255
#define HOST_NONE "NONE"
#define HOST_REDIRECT "RED"

typedef struct TrackInfo_st
{
	char *TrackName;
	char *FwdHost;
	uint FwdPortNo;
}
TrackInfo_t;

static autoList_t *TrackInfoList;

static void AddTrackInfo(char *trackName, char *fwdHost, char *sFwdPortNo)
{
	TrackInfo_t *i = nb_(TrackInfo_t);

	errorCase(m_isEmpty(trackName) || TRACKNAME_LENMAX < strlen(trackName));
	errorCase(m_isEmpty(fwdHost) || HOST_LENMAX < strlen(fwdHost));
	errorCase(m_isEmpty(sFwdPortNo));

	i->TrackName = strx(trackName);
	i->FwdHost = strx(fwdHost);
	i->FwdPortNo = toValue(sFwdPortNo);

	errorCase(!i->FwdPortNo || 0xffff < i->FwdPortNo);

	cout("TrackName: %s\n", i->TrackName);
	cout("FwdHost: %s\n", i->FwdHost);
	cout("FwdPortNo: %u\n", i->FwdPortNo);

	addElement(TrackInfoList, (uint)i);
}

static void PreDataFltr(autoBlock_t *buff, uint uPData)
{
	char **pData = (char **)uPData;

	if (*pData)
	{
		autoBlock_t *b = newBlock();

		ab_addLine(b, *pData);
		ab_addBytes(b, buff);

		ab_swap(b, buff);

		releaseAutoBlock(b);

		*pData = NULL;
	}
}
static void TransmitTh(int sock, char *fwdHost, uint fwdPortNo, char *data)
{
	uchar ip[4] = { 0 };
	int fwdSock;

	cout("%s %s(%u) %d\n", c_makeJStamp(NULL, 0), fwdHost, fwdPortNo, data ? (sint)strlen(data) : -1);

	fwdSock = sockConnect(ip, fwdHost, fwdPortNo);

	if (fwdSock == -1)
		return;

	CrossChannel(sock, fwdSock, PreDataFltr, (uint)&data, NULL, 0);
	sockDisconnect(fwdSock);
}
static void PerformTh(int sock, char *strip)
{
	char *buff = strx("");
	uint abortTime = now() + (IsTight() ? 2 : 60);
	char *trackName = NULL;

	for (; ; )
	{
		char cBuff[1];
		int retval;

		retval = SockTransmit(sock, cBuff, 1, 100, 0);

		if (retval == -1)
			goto disconnect;

		if (retval == 1)
		{
			if (cBuff[0] == 0x00) // HACK: �g���b�N���̕����� '\0' ������ꍇ�͖��Ή� -> �f�t�H���g�]�������ɐؒf
				goto disconnect;

			buff = addChar(buff, cBuff[0]);

			if (endsWith(buff, "\r\n"))
			{
				TrackInfo_t *i;
				uint index;

				trackName = strx(buff);
				strchr(trackName, '\0')[-2] = '\0';

				foreach (TrackInfoList, i, index)
				{
					if (!_stricmp(i->TrackName, trackName))
					{
						TransmitTh(sock, i->FwdHost, i->FwdPortNo, NULL);
						goto disconnect;
					}
				}
				break;
			}
			if (TRACKNAME_LENMAX + 2 <= strlen(buff))
				break;
		}
		if (abortTime < now())
			break;
	}

	{
		char *tmp = lineToPrintLine(buff, 0);
		cout("�g���b�N�U�蕪�����s_buff: [%s]\n", tmp);
		memFree(tmp);
	}

	if (!strcmp(FwdHost, HOST_NONE))
	{
		cout("�f�t�H���g�̓]����͖����ł��B\n");
	}
	else if (!strcmp(FwdHost, HOST_REDIRECT))
	{
		char *fwdHost;
		uint fwdPortNo;

		if (!trackName)
			goto disconnect;

		fwdHost = trackName;

		if (*fwdHost)
			line2csym_ext(fwdHost, "-.:");

		{
			char *p = strchr(fwdHost, ':');

			if (p)
			{
				*p = '\0';
				fwdPortNo = toValue(p + 1);
			}
			else
			{
				fwdPortNo = FwdPortNo;
			}
		}

		cout("���_�C���N�g���ꂽ�]����� [%s] �|�[�g�ԍ� %u �ł��B\n", fwdHost, fwdPortNo);

		if (*fwdHost && m_isRange(fwdPortNo, 1, 65535))
			TransmitTh(sock, fwdHost, fwdPortNo, NULL);
		else
			cout("���]����ɖ�肪���邽�߁A�ؒf���܂��B\n");
	}
	else
	{
		TransmitTh(sock, FwdHost, FwdPortNo, buff);
	}

disconnect:
	memFree(buff);
	memFree(trackName);
}
static int ReadArgs(void)
{
	if (argIs("/F"))
	{
		autoList_t *lines = readResourceLines(nextArg());
		char *line;
		uint index;

		foreach (lines, line, index)
		{
			autoList_t *tokens = ucTokenize(line);

			errorCase(getCount(tokens) != 3);

			AddTrackInfo(
				getLine(tokens, 0),
				getLine(tokens, 1),
				getLine(tokens, 2)
				);

			releaseDim(tokens, 1);
		}
		releaseDim(lines, 1);
		return 1;
	}
	if (hasArgs(3))
	{
		AddTrackInfo(
			getArg(0),
			getArg(1),
			getArg(2)
			);

		skipArg(3);
		return 1;
	}

	if (!strcmp(FwdHost, HOST_NONE))
	{
		cout("+---------------------------------------+\n");
		cout("| �f�t�H���g�]���͖����Ɏw�肳��܂����B|\n");
		cout("+---------------------------------------+\n");
	}
	else if (!strcmp(FwdHost, HOST_REDIRECT))
	{
		cout("+-------------------------------------------------------+\n");
		cout("| �f�t�H���g�]���̓��_�C���N�g�E���[�h�Ɏw�肳��܂����B|\n");
		cout("+-------------------------------------------------------+\n");
	}

	return 0;
}
static char *GetTitleSuffix(void)
{
	return xcout("F:%u", getCount(TrackInfoList));
}
int main(int argc, char **argv)
{
	TrackInfoList = newList();

	TunnelPerformTh = PerformTh;
	TunnelMain(ReadArgs, NULL, "multiTrack", GetTitleSuffix);
	TunnelPerformTh = NULL;
}
