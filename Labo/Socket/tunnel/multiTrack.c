/*
	multiTrack.exe RECV-PORT DEF-FWD-HOST DEF-FWD-PORT /S

		/S ... 停止する。

	multiTrack.exe RECV-PORT DEF-FWD-HOST DEF-FWD-PORT [/C CONNECT-MAX] [TRACK-NAME FWD-HOST FWD-PORT | /F FWD-FILE]...

		CONNECT-MAX ... 最大接続数, 省略時は 1000
		TRACK-NAME ... トラック名
		FWD-HOST ... 転送先ホスト名
		FWD-PORT ... 転送先ポート番号
		FWD-FILE ... 転送先リスト定義ファイル

		★ DEF-FWD-HOST に NONE を指定すると、デフォルトの転送先は無効になります。

		★ DEF-FWD-HOST に RED を指定すると、リダイレクト・モード（トラック名が転送先）になります。

	- - - -

	転送先リスト定義ファイル

		テキストファイルで、空行と半角セミコロンで始まる行は無視します。
		各行は以下のように記述します。

			ホスト名 + 空白 + 転送先ホスト名 + 空白 + 転送先ポート番号
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
			if (cBuff[0] == 0x00) // HACK: トラック名の部分に '\0' がある場合は未対応 -> デフォルト転送せずに切断
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
		cout("トラック振り分け失敗_buff: [%s]\n", tmp);
		memFree(tmp);
	}

	if (!strcmp(FwdHost, HOST_NONE))
	{
		cout("デフォルトの転送先は無効です。\n");
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

		cout("リダイレクトされた転送先は [%s] ポート番号 %u です。\n", fwdHost, fwdPortNo);

		if (*fwdHost && m_isRange(fwdPortNo, 1, 65535))
			TransmitTh(sock, fwdHost, fwdPortNo, NULL);
		else
			cout("★転送先に問題があるため、切断します。\n");
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
		cout("| デフォルト転送は無効に指定されました。|\n");
		cout("+---------------------------------------+\n");
	}
	else if (!strcmp(FwdHost, HOST_REDIRECT))
	{
		cout("+-------------------------------------------------------+\n");
		cout("| デフォルト転送はリダイレクト・モードに指定されました。|\n");
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
