/*
	非NTPでシステム時刻合わせ。
*/

#include "C:\Factory\Common\Options\SClient.h"
#include "C:\Factory\Labo\Socket\libs\http\Chunked.h"
#include "libs\ApplyStampData.h"

#define NICT_DOMAIN "ntp-a1.nict.go.jp"
//#define NICT_DOMAIN "ntp-b1.nict.go.jp" // 予備の鯖？
#define NICT_PORT 80

static time_t NictTime;

static int NictPerform(int sock, uint prm_dummy)
{
	SockStream_t *ss = CreateSockStream(sock, 5);
	char *line;
	int ret = 0;
	int chunked = 0;

	SockSendLine_NF(ss, "GET /cgi-bin/jst HTTP/1.1");
	SockSendLine_NF(ss, "Host: " NICT_DOMAIN);
	SockSendLine(ss, "");

	for (; ; ) // ヘッダ読み込み
	{
		char *line = SockRecvLine(ss, 2000);

		if (!*line)
		{
			memFree(line);
			break;
		}
		removeBlank(line);

		if (!_stricmp(line, "Transfer-Encoding:chunked"))
			chunked = 1;

		memFree(line);
	}

	if (chunked)
	{
		httpChunkedRecver_t *recver = httpCreateChunkedRecver(ss);
		autoBlock_t *body;

		body = httpRecvChunked(recver);

		if (body)
		{
			char *sBody = unbindBlock2Line(body);
			char *p;
			char *q;

			p = stristrNext(sBody, "<BODY>");
			p = m_noz(p, sBody);
			q = stristrEnd(p, "</BODY>");
			*q = '\0';

			line = strx(p);

			memFree(sBody);

			removeBlank(line);
		}
		else
		{
			line = strx("<NULL>");
		}

		httpReleaseChunkedRecver(recver);
	}
	else
	{
		memFree(SockRecvLine(ss, 100)); // <HTML>
		memFree(SockRecvLine(ss, 100)); // <HEAD><TITLE> ...
		memFree(SockRecvLine(ss, 100)); // <BODY>

		line = SockRecvLine(ss, 100);
	}

	line2JLine(line, 1, 0, 0, 1); // 表示のため
	cout("[%s]\n", line);

	if (
		lineExp("<1,,09>", line) ||
		lineExp("<1,,09>.<1,,09>", line)
		)
	{
		*strchrEnd(line, '.') = '\0'; // 小数点以下を除去

		NictTime = toValue64(line);

		if (m_isRange(NictTime, 1000000000ui64, 32500000000ui64)) // ? およそ 2000年 ～ 3000年, それ以外は何かおかしい！
			ret = 1;
	}
	ReleaseSockStream(ss);
	memFree(line);
	return ret;
}
static int GetNictTime(void) // ret: ? 成功
{
	int ret;

	// HACK: 秒単位で応答が遅くなれば、それだけ不正確な日時になるよなぁ...
	// -- Slewで日時セットしているので時々そういうことがあってもいいや...

	LOGPOS();
	ret = SClient(NICT_DOMAIN, NICT_PORT, NictPerform, 0);
	LOGPOS();

	return ret;
}
static void Main2(void)
{
	int viewOnly = 0;
	int dayChangeEvasion = 1;

readArgs:
	if (argIs("/V"))
	{
		viewOnly = 1;
		goto readArgs;
	}
	if (argIs("/-E"))
	{
		dayChangeEvasion = 0;
		goto readArgs;
	}

	// ----

	LOGPOS();

	// ss.000 秒になるべく近づけてみる。
	{
		time_t t = time(NULL);

		do
		{
			sleep(10);
		}
		while (t == time(NULL));
	}

	LOGPOS();

	if (dayChangeEvasion)
	{
		uint hms = (uint)(toValue64(c_makeCompactStamp(NULL)) % 1000000ui64);

		cout("hms: %06u\n", hms);

		// アクティブオープンに最長 20 sec, 通信タイムアウト 5 sec で、だいたい 25 秒 + 2～3 秒くらいで更新を掛けられるはず。
		// 余裕は 35 秒くらいでいいと思う。
		// -- 1 分でいいや。

//		if (hms < 5 || 235925 < hms)
		if (hms < 5 || 235900 < hms)
		{
			cout("日付変更が近いので、中止します。\n");
			return;
		}
	}
	if (!GetNictTime())
	{
		cout("取得失敗\n");
		return;
	}
	cout("取得した時刻：%s\n", c_makeJStamp(getStampDataTime(NictTime), 0));

	if (!viewOnly)
	{
		LOGPOS();
		SlewApplyTimeData(NictTime);
		LOGPOS();
	}
}
int main(int argc, char **argv)
{
	Main2();
	termination(0);
}
