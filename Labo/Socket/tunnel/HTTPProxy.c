/*
	HTTPProxy.exe RECV-PORT FWD-HOST FWD-PORT /S

		/S ... 停止する。

	HTTPProxy.exe RECV-PORT FWD-HOST FWD-PORT [/C CONNECT-MAX] [/T SOCK-TIMEOUT] [/MT MESSAGE-TIMEOUT]
	                                          [/FBT U-FIRST-BYTE-TIMEOUT] [/FBT- D-FIRST-BYTE-TIMEOUT]
	                                          [/NDT U-NO-DATA-TIMEOUT] [/NDT- D-NO-DATA-TIMEOUT]
	                                          [/M MESSAGE-MAX] [/X] [/D] [/-D] [/+C]
	                                          [/IP OK-IP-PFX]... [/-IP NG-IP-PFX]...
	                                          [/HR H-KEY H-VAL]... [/HK H-KEY]...
	                                          [/HR- H-KEY H-VAL]... [/HK- H-KEY]...
	                                          [/HR+ H-KEY H-VAL]... [/HK+ H-KEY]...
	                                          [/F FILTER-COMMAND]
	                                          [/F- FILTER-COMMAND]
	                                          [/F+ FILTER-COMMAND]

		CONNECT-MAX          ... 最大接続数, 省略時 100
		SOCK-TIMEOUT         ... 通信タイムアウト [秒] 0 == 無制限, 省略時 10 日
		U-FIRST-BYTE-TIMEOUT ... 上り最初の１バイトのタイムアウト [秒] 0 == 無制限, 省略時 3 秒
		D-FIRST-BYTE-TIMEOUT ... 下り最初の１バイトのタイムアウト [秒] 0 == 無制限, 省略時 5 秒
		U-NO-DATA-TIMEOUT    ... 上り無通信タイムアウト [秒] 0 == 無制限, 省略時 3 分
		D-NO-DATA-TIMEOUT    ... 下り無通信タイムアウト [秒] 0 == 無制限, 省略時 5 分
		MESSAGE-TIMEOUT      ... １回(１つ)のメッセージの送信・受信タイムアウト [秒] 0 == 無制限, 省略時 10 時間
		MESSAGE-MAX          ... メッセージバッファの最大合計サイズ, 省略時は 270 MB
		/X  ... Hostタグを見て転送先を変更する。
		/D  ... URLにスキーム・ドメインを挿入する。
		/-D ... URLからスキーム・ドメインを除去する。
		/+C ... CONNECTメソッドを許可する。(https等を許可する)
		OK-IP-PFX ... 転送先として許可するIPアドレスの開始パターン
		NG-IP-PFX ... 転送先として許可しないIPアドレスの開始パターン
		/HR  ... ヘッダフィールドの置き換え・追加 (上りのみ)
		/HK  ... ヘッダフィールドの削除           (上りのみ)
		/HR- ... ヘッダフィールドの置き換え・追加 (下りのみ)
		/HK- ... ヘッダフィールドの削除           (下りのみ)
		/HR+ ... ヘッダフィールドの置き換え・追加 (上下両方)
		/HK+ ... ヘッダフィールドの削除           (上下両方)
		/F   ... フィルターコマンド (上りのみ)
		/F-  ... フィルターコマンド (下りのみ)
		/F+  ... フィルターコマンド (上下両方)

	特殊なH-VAL

		$$ で始まる場合、先頭の $$ を除去します。

			例：  $$$100-  ->  $100-

		$H で始まる場合、３文字目以降をヘッダーフィールド名と見なし、その値に置き換えます。

			例：  $HHost  ->  stackprobe.ccsp.mydns.jp:8080

	フィルターコマンド

		system関数にそのまま渡せる文字列であること。
		コマンド実行時のカレントは、このプロセス実行時のカレントと同じ。
		実行時のカレントに HTTPParse_HttpDat.tmp というファイルにヘッダとボディが書き出されているので、
		これを変更すると、送受信データに反映される。

		コマンド引数がある場合は "" で囲ってね。

			例：  /f- "HTTPPBodyFilter.exe image/ image/png sample\0005\Face.png"

	----

	2015.1.7 SEGV -> PfmConnect, buffTmp1 が複製されていなかったため ReallocSessionBuffs の再配置で変になったのかも？
*/

#include "libs\Tunnel.h"
#include "libs\HTTPParse.h"
#include "C:\Factory\Common\Options\SockStream.h"

// ---- Prms ----

static uint SockTimeoutSec = 864000;
static uint U_FirstByteTimeoutSec = 3;
static uint D_FirstByteTimeoutSec = 5;
static uint U_NoDataTimeoutSec = 180;
static uint D_NoDataTimeoutSec = 300;
static uint MessageTimeoutSec = 36000;
static uint MessageTotalSizeMax = 270000000;
static int ChangeFwdMode;
static int InsDomainMode;
static int DelDomainMode;
static int ConnectMethodOkMode;
static autoList_t *OKIPPrfxList; // "" を含まない
static autoList_t *NGIPPrfxList; // "" を含まない
static autoList_t *UR_Keys;
static autoList_t *UR_Values;
static autoList_t *UK_Keys;
static autoList_t *DR_Keys;
static autoList_t *DR_Values;
static autoList_t *DK_Keys;
static char *UploadFilterCommand;
static char *DownloadFilterCommand;

// ----

typedef struct Session_st
{
	int Sock;
	int FwdSock;
	char *FwdName;
	autoBlock_t *Buff;
	autoBlock_t *BkBuff;
}
Session_t;

static Session_t *SessionList;
static autoList_t *SessionStock;

// ★★★ RecvHTTPParse ★★★

static void ReallocBuff(autoBlock_t **pBuff)
{
	errorCase(!ConnectMax); // 2bs: 起動時にエラーになるはず。

	if (getSize(*pBuff) < MessageTotalSizeMax / ConnectMax) // ? 適当に小さいの
	{
		autoBlock_t *tmp = copyAutoBlock(*pBuff);
		releaseAutoBlock(*pBuff);
		*pBuff = tmp;
	}
}
static void ReallocSessionBuffs(void)
{
	uint index;

	LOGPOS();

	for (index = 0; index < ConnectMax; index++)
	{
		Session_t *i = SessionList + index;

		ReallocBuff(&i->Buff);
		ReallocBuff(&i->BkBuff);
	}
	LOGPOS();
}
static uint GetMessageTotalSize(void)
{
	uint totalSize = 0;
	uint index;

	for (index = 0; index < ConnectMax; index++)
	{
		Session_t *i = SessionList + index;

		totalSize += getSize(i->Buff);
		totalSize += getSize(i->BkBuff);
	}
	return totalSize;
}
static int RecvHTTPParse(Session_t *i, int sock, uint firstByteTmoutSec, uint noDatTmoutSec, int oSSock) // ret: ? 成功
{
	SockStream_t *ss = CreateSockStream(sock, noDatTmoutSec);
	SockStream_t *osss = oSSock != -1 ? CreateSockStream(oSSock, 0) : NULL;
	int retval = 0;
	int needTryParse = (int)getSize(i->BkBuff);
	int needBreak = 0;
	uint firstByteTmoutTime = GetTimeoutTime(firstByteTmoutSec);

	for (; ; )
	{
		if (osss && !getSize(i->BkBuff) && SockRecvCharWait(osss, 0))
		{
			int chr = SockRecvChar(osss);

			if (chr == EOF)
			{
				cout("対岸から切断されました。\n");
				break;
			}
			addByte(i->BkBuff, chr); // memo: 片方から受信中に、もう片方からデータが送信されることってあるのか？
		}

		while (SockRecvCharWait(ss, needTryParse ? 0 : 2000))
		{
			int chr = SockRecvChar(ss);

			if (chr == EOF)
			{
				needBreak = 1;
				break;
			}
			firstByteTmoutTime = UINTMAX; // 少なくとも１バイト受信したので無効化する。
			needTryParse = 1;
			addByte(i->Buff, chr);

			{
				static uint rCnt;

				rCnt++;
				rCnt %= 65536;

				if (!rCnt)
				{
					ReallocSessionBuffs();

					if (MessageTotalSizeMax < GetMessageTotalSize())
					{
						cout("+--------------------------------------------------+\n");
						cout("| メッセージバッファサイズの合計が上限に達しました |\n");
						cout("+--------------------------------------------------+\n");
						goto endFunc;
					}

					{
						static uint rCnt2;

						rCnt2++;
						rCnt2 %= 16;

						if (!rCnt2)
						{
							break; // パース等強制的に実施する。
						}
					}
				}
			}
		}

		if (HTTPParse(i->Buff))
		{
			if (HttpDat.Expect100Continue)
			{
				cout("Expect: 100-continue には対応していません。\n");
				break;
			}

			// ProcHTTP(), MakeSendData() で HttpDat を触る。
			// ここから MakeSendData() が終わるまでの間にスレッドが切り替わるとまずい。
			// <- 別スレッドの HTTPParse() によって HttpDat が変更されるため

			removeBytes(i->Buff, 0, HttpDat.EndPos); // パース済みの受信データを削除
			retval = 1;
			break;
		}

		if (needBreak)
			break;

		if (ProcDeadFlag)
		{
			cout("プロセスの終了による受信キャンセル\n");
			break;
		}
		if (firstByteTmoutTime < now())
		{
			cout("最初の壱バイトのタイムアウト\n");
			break;
		}

		needTryParse = 0;
		needBreak = 0;
	}

endFunc:
	ab_addBytes_x(i->Buff, SockRipRecvBuffer(ss));
	ReleaseSockStream(ss);

	if (osss)
	{
		ab_addBytes_x(i->BkBuff, SockRipRecvBuffer(osss));
		ReleaseSockStream(osss);
	}
	return retval;
}

// ★★★ ProcHTTP ★★★

static char *HeaderRepValueFltr(char *value) // ret: strx()
{
	if (*value == '$')
	{
		switch (c2upper(value[1]))
		{
		case '$':
			value += 2;
			break;

		case 'H':
			value = refLine(HttpDat.H_Values, findLineCase(HttpDat.H_Keys, value + 2, 1));
			break;
		}
	}
	return strx(value);
}
static void HeaderRepKill(autoList_t *r_keys, autoList_t *r_values, autoList_t *k_keys)
{
	char *key;
	uint index;

	foreach (r_keys, key, index)
	{
		uint hPos = findLineCase(HttpDat.H_Keys, key, 1);
		char *r_value = getLine(r_values, index);

		cout("r_val_ORIG: [%s]\n", r_value);
		r_value = HeaderRepValueFltr(r_value);

		cout("R_Key: [%s]\n", key);

		if (hPos == getCount(HttpDat.H_Keys))
		{
			addElement(HttpDat.H_Keys, (uint)strx(key));
			addElement(HttpDat.H_Values, (uint)NULL);

			cout("R_Val: noDef -> [%s]\n", r_value);
		}
		else
		{
			cout("R_Val: [%s] -> [%s]\n", getLine(HttpDat.H_Values, hPos), r_value);
		}
		strzp_x((char **)directGetPoint(HttpDat.H_Values, hPos), r_value);
	}
	foreach (k_keys, key, index)
	{
		uint hPos = findLineCase(HttpDat.H_Keys, key, 1);

		cout("K_Key: [%s]\n", key);

		if (hPos < getCount(HttpDat.H_Keys))
		{
			cout("K_Val: [%s]\n", getLine(HttpDat.H_Values, hPos));

			memFree((char *)desertElement(HttpDat.H_Keys, hPos));
			memFree((char *)desertElement(HttpDat.H_Values, hPos));
		}
		else
		{
			cout("K_Val: noDef\n");
		}
	}
}
static int DoConnect(char *fwdHost, uint fwdPortNo) // ret: -1 == 接続できなかった。
{
	uchar ip[4];
	char *strIp;
	char *ipprfx;
	uint index;

	cout("FWD_HOST: %s\n", fwdHost);
	cout("FWD_PORT: %u\n", fwdPortNo);

	if (!m_isRange(fwdPortNo, 1, 65535))
	{
		cout("★ポート番号に問題があります。\n");
		return -1;
	}
	sockLookup(ip, fwdHost);
	strIp = SockIp2Line(ip);
	cout("FWD_IP: %s\n", strIp);

	if (!*(uint *)ip)
	{
		cout("★転送先が見つかりません。\n");
		return -1;
	}
	if (getCount(OKIPPrfxList))
	{
		foreach (OKIPPrfxList, ipprfx, index)
		{
			if (startsWith(strIp, ipprfx))
				break;
		}
		if (!ipprfx)
		{
			cout("●許可されたIPアドレスではありません。\n");
			return -1;
		}
	}
	foreach (NGIPPrfxList, ipprfx, index)
	{
		if (startsWith(strIp, ipprfx))
		{
			cout("■許可されないIPアドレスです。\n");
			return -1;
		}
	}

	{
		HttpDat_t backup;
		int sock;

		backup = EjectHttpDat();
		sock = sockConnect_NB(ip, NULL, fwdPortNo);
		UnejectHttpDat(backup);

		return sock;
	}
}
static void DoConnect_HD(Session_t *i, uint defPortNo)
{
	if (ChangeFwdMode)
	{
		char *host = strx(refLine(HttpDat.H_Values, findLineCase(HttpDat.H_Keys, "Host", 1)));
		uint portNo;
		char *p;
		char *name;

		p = strchr(host, ':');

		if (p)
		{
			*p = '\0';
			p++;

			portNo = toValue(p);
		}
		else
		{
			portNo = defPortNo;
		}
		name = xcout("%s:%u", host, portNo);

		if (strcmp(name, i->FwdName)) // ? 前回の接続先と異なる。|| 未接続
		{
			if (i->FwdSock != -1)
				sockDisconnect(i->FwdSock);

			i->FwdSock = DoConnect(host, portNo);
			memFree(i->FwdName);
			i->FwdName = strx(name);
		}
		else
		{
			cout("KeepConn_1\n");
		}
		memFree(host);
		memFree(name);
	}
	else
	{
		if (i->FwdSock == -1) // ? 未接続
			i->FwdSock = DoConnect(FwdHost, FwdPortNo);
		else
			cout("KeepConn_2\n");
	}
}
static int ProcHTTP_Upload(Session_t *i) // ret: ? 成功
{
	HeaderRepKill(UR_Keys, UR_Values, UK_Keys);

	// 接続
	{
		DoConnect_HD(i, 80);

		if (i->FwdSock == -1)
		{
			cout("+------------------------------+\n");
			cout("| 転送先に接続できませんでした |\n");
			cout("+------------------------------+\n");
			return 0; // 失敗
		}
	}

	if (InsDomainMode)
	{
		char *p = strchr(HttpDat.H_Request, ' ');

		if (p && p[1] == '/') // ? スキーム・ドメインが無いっぽい。
		{
			char *hostName = strx(refLine(HttpDat.H_Values, findLineCase(HttpDat.H_Keys, "Host", 1)));
			char *insPtn;

			insPtn = xcout("http://%s", hostName);

			cout("InsDomain\n");
			cout("< [%s]\n", HttpDat.H_Request);

			HttpDat.H_Request = insertLine(HttpDat.H_Request, ((uint)p + 1) - (uint)HttpDat.H_Request, insPtn);

			cout("> [%s]\n", HttpDat.H_Request);

			memFree(hostName);
			memFree(insPtn);
		}
	}

	if (DelDomainMode)
	{
		char *p = strchr(HttpDat.H_Request, ' ');
		char *q;

		if (p && p[1] != '/')
		{
			p++;
			q = strstr(p, "//");

			if (q)
			{
				q = strchr(q + 2, '/');

				if (q) // ? スキーム・ドメインが有るっぽい。
				{
					cout("DelDomain\n");
					cout("< [%s]\n", HttpDat.H_Request);

					copyLine(p, q);

					cout("> [%s]\n", HttpDat.H_Request);
				}
			}
		}
	}

	return 1; // 成功
}
static int ProcHTTP_Download(Session_t *i) // ret: ? 成功
{
	HeaderRepKill(DR_Keys, DR_Values, DK_Keys);

	return 1; // 成功
}

// ★★★ SendHTTP ★★★

static char *HFldFolding(char *str)
{
	uint index = 50;

	while (index + 10 <= strlen(str))
	{
		if (str[index] <= '\x20')
		{
			str = insertLine(str, index, "\r\n");
			index += 50;
		}
		index++;
	}
	return str;
}
static autoBlock_t *MakeSendData(void)
{
	autoBlock_t *buff = newBlock();

	ab_addLine(buff, HttpDat.H_Request);
	ab_addLine(buff, "\r\n");

	{
		uint index;

		for (index = 0; index < getCount(HttpDat.H_Keys); index++)
		{
			char *value = getLine(HttpDat.H_Values, index);

			value = strx(value);
//			value = HFldFolding(value);

			ab_addLine(buff, getLine(HttpDat.H_Keys, index));
			ab_addLine(buff, ": ");
			ab_addLine(buff, value);
			ab_addLine(buff, "\r\n");

			memFree(value);
		}
	}

	ab_addLine(buff, "\r\n");

	if (HttpDat.Chunked)
	{
		uint rPos = 0;

		while (rPos < getSize(HttpDat.Body))
		{
			uint partSize = m_min(0xfff, getSize(HttpDat.Body) - rPos);

			ab_addLine_x(buff, xcout("%x\r\n", partSize));
			ab_addSubBytes(buff, HttpDat.Body, rPos, partSize);
			ab_addLine(buff, "\r\n");

			rPos += partSize;
		}
		ab_addLine(buff, "0\r\n\r\n");
	}
	else
	{
		ab_addBytes(buff, HttpDat.Body);
	}

	// test
	/*
	{
		char *tmp;
		cout("SENDBuff:[%s]\n", tmp = toPrintLine(buff, 1));
		memFree(tmp);
	}
	*/
	return buff;
}
static int SendHTTP(Session_t *i, int sock, uint noDatTmoutSec) // ret: ? 通信エラー
{
	autoBlock_t *sendData = MakeSendData();
	uint endTime = GetTimeoutTime(MessageTimeoutSec);
	int retval = 0;
	int ret;
	uint noDatTmoutTime = GetTimeoutTime(noDatTmoutSec);

	for (; ; )
	{
		if (!getSize(sendData))
		{
			cout("送信完了\n");
			retval = 1;
			break;
		}
		if (endTime < now())
		{
			cout("送信タイムアウト\n");
			break;
		}
		ret = SockSendSequ(sock, sendData, 2000);

		if (ret == -1)
		{
			cout("送信エラー\n");
			break;
		}
		if (ret)
		{
			noDatTmoutTime = GetTimeoutTime(noDatTmoutSec);
		}
		else if (noDatTmoutTime < now())
		{
			cout("無通信タイムアウト！\n");
			break;
		}
		if (ProcDeadFlag)
		{
			cout("送信中止！(ProcDeadFlag)\n");
			break;
		}
	}
	releaseAutoBlock(sendData);
	return retval;
}

// ★★★ ここまで ★★★

static void PreDataFltr(autoBlock_t *buff, uint uPData)
{
	autoBlock_t **pData = (autoBlock_t **)uPData;

	if (*pData)
	{
		autoBlock_t *nb = newBlock();

		ab_addBytes(nb, *pData);
		ab_addBytes(nb, buff);

		ab_swap(nb, buff);

		releaseAutoBlock(nb);

		*pData = NULL;
	}
}
static void PfmConnect(Session_t *i)
{
	autoBlock_t *sendData = newBlock();

	if (i->FwdSock != -1) // ? 接続中 -> 切断
	{
		sockDisconnect(i->FwdSock);
		i->FwdSock = -1;
	}
	DoConnect_HD(i, 443);

	if (i->FwdSock != -1) // ? 接続成功
	{
		autoBlock_t *iBuffTmp = copyAutoBlock(i->Buff); // i->Buff は ReallocSessionBuffs() で触られるので複製する。
		autoBlock_t *buffTmp1;
		autoBlock_t *buffTmp2 = sendData;

		buffTmp1 = iBuffTmp;
		ab_addLine(sendData, "HTTP/1.1 200 Blue Mountain\r\n\r\n");

		cout("CC_Begin\n");
		CrossChannel(i->Sock, i->FwdSock, PreDataFltr, (uint)&buffTmp1, PreDataFltr, (uint)&buffTmp2);
		cout("CC_End\n");

		releaseAutoBlock(iBuffTmp);
	}
	else // ? 接続失敗
	{
		ab_addLine(sendData, "HTTP/1.1 404 Emerald Mountain\r\n\r\n");
		SockSendSequLoop(i->Sock, sendData, 2000);
	}
	releaseAutoBlock(sendData);
}
static void DoFilterCommand(char *filterCommand)
{
	if (!filterCommand)
		return;

	mutex();
	{
		SaveHttpDat(DEF_HTTP_DAT_FILE);
		execute(filterCommand);
		LoadHttpDat(DEF_HTTP_DAT_FILE);
		removeFile(DEF_HTTP_DAT_FILE);
	}
	unmutex();
}
static char *FilterCommandFltr(char *filterCommand)
{
	return filterCommand;
}
static void PerformTh(int sock, char *strip)
{
	Session_t *i = (Session_t *)unaddElement(SessionStock);
	uint endTime = now() + SockTimeoutSec;

	cout("接続 %d\n", sock);

	i->Sock = sock;
	i->FwdSock = -1; // 2bs
	i->FwdName[0] = '\0'; // 2bs
	setSize(i->Buff, 0); // 2bs
	setSize(i->BkBuff, 0); // 2bs

	while (now() <= endTime)
	{
		// ---- 上り ----

		if (!RecvHTTPParse(i, i->Sock, U_FirstByteTimeoutSec, U_NoDataTimeoutSec, i->FwdSock))
			break;

		if (
			ConnectMethodOkMode &&
			startsWithICase(HttpDat.H_Request, "CONNECT\x20")
			)
		{
			PfmConnect(i);
			break;
		}
		DoFilterCommand(UploadFilterCommand);

		if (!ProcHTTP_Upload(i))
			break;

		if (!SendHTTP(i, i->FwdSock, U_NoDataTimeoutSec))
			break;

		ab_swap(i->Buff, i->BkBuff);

		// ---- 下り ----

		if (!RecvHTTPParse(i, i->FwdSock, D_FirstByteTimeoutSec, D_NoDataTimeoutSec, i->Sock))
			break;

		DoFilterCommand(DownloadFilterCommand);

		if (!ProcHTTP_Download(i))
			break;

		if (!SendHTTP(i, i->Sock, D_NoDataTimeoutSec))
			break;

		ab_swap(i->Buff, i->BkBuff);

		// ----
	}

	if (i->FwdSock != -1)
	{
//		LOGPOS();
		sockDisconnect(i->FwdSock);
	}
	i->Sock = -1;
	i->FwdSock = -1;
	i->FwdName[0] = '\0';
	setSize(i->Buff, 0);
	setSize(i->BkBuff, 0);

	cout("切断 %d\n", sock);

	addElement(SessionStock, (uint)i);
}
static int ReadArg_HR(autoList_t *r_keys, autoList_t *r_values)
{
	addElement(r_keys, (uint)nextArg());
	addElement(r_values, (uint)nextArg());
	return 1;
}
static int ReadArg_HK(autoList_t *k_keys)
{
	addElement(k_keys, (uint)nextArg());
	return 1;
}
static int ReadArgs(void)
{
	if (argIs("/T"))
	{
		SockTimeoutSec = toValue(nextArg());
		return 1;
	}
	if (argIs("/FBT")) // 上り最初の１バイトのタイムアウト
	{
		U_FirstByteTimeoutSec = toValue(nextArg());
		return 1;
	}
	if (argIs("/FBT-")) // 下り最初の１バイトのタイムアウト
	{
		D_FirstByteTimeoutSec = toValue(nextArg());
		return 1;
	}
	if (argIs("/NDT")) // 上り無通信タイムアウト
	{
		U_NoDataTimeoutSec = toValue(nextArg());
		return 1;
	}
	if (argIs("/NDT-")) // 下り無通信タイムアウト
	{
		D_NoDataTimeoutSec = toValue(nextArg());
		return 1;
	}
	if (argIs("/MT"))
	{
		MessageTimeoutSec = toValue(nextArg());
		return 1;
	}
	if (argIs("/M"))
	{
		MessageTotalSizeMax = toValue(nextArg());
		return 1;
	}
	if (argIs("/X"))
	{
		ChangeFwdMode = 1;
		return 1;
	}
	if (argIs("/D"))
	{
		InsDomainMode = 1;
		return 1;
	}
	if (argIs("/-D"))
	{
		DelDomainMode = 1;
		return 1;
	}
	if (argIs("/+C"))
	{
		ConnectMethodOkMode = 1;
		return 1;
	}
	if (argIs("/IP"))
	{
		char *ipprfx = nextArg();

		errorCase(!*ipprfx); // ? ""

		addElement(OKIPPrfxList, (uint)ipprfx);
		return 1;
	}
	if (argIs("/-IP"))
	{
		char *ipprfx = nextArg();

		errorCase(!*ipprfx); // ? ""

		addElement(NGIPPrfxList, (uint)ipprfx);
		return 1;
	}
	if (argIs("/HR"))
	{
		return ReadArg_HR(UR_Keys, UR_Values);
	}
	if (argIs("/HK"))
	{
		return ReadArg_HK(UK_Keys);
	}
	if (argIs("/HR-"))
	{
		return ReadArg_HR(DR_Keys, DR_Values);
	}
	if (argIs("/HK-"))
	{
		return ReadArg_HK(DK_Keys);
	}
	if (argIs("/HR+"))
	{
		return
			ReadArg_HR(UR_Keys, UR_Values) &&
			ReadArg_HR(DR_Keys, DR_Values);
	}
	if (argIs("/HK+"))
	{
		return
			ReadArg_HK(UK_Keys) &&
			ReadArg_HK(DK_Keys);
	}
	if (argIs("/F"))
	{
		UploadFilterCommand = FilterCommandFltr(nextArg());
		return 1;
	}
	if (argIs("/F-"))
	{
		DownloadFilterCommand = FilterCommandFltr(nextArg());
		return 1;
	}
	if (argIs("/F+"))
	{
		char *command = FilterCommandFltr(nextArg());

		UploadFilterCommand = command;
		DownloadFilterCommand = command;
		return 1;
	}

	cout("ConnectMax: %u\n", ConnectMax);
	cout("SockTimeoutSec: %u\n", SockTimeoutSec);
	cout("U_FirstByteTimeoutSec: %u\n", U_FirstByteTimeoutSec);
	cout("D_FirstByteTimeoutSec: %u\n", D_FirstByteTimeoutSec);
	cout("U_NoDataTimeoutSec: %u\n", U_NoDataTimeoutSec);
	cout("D_NoDataTimeoutSec: %u\n", D_NoDataTimeoutSec);
	cout("MessageTimeoutSec: %u\n", MessageTimeoutSec);
	cout("MessageTotalSizeMax: %u\n", MessageTotalSizeMax);
	cout("ChangeFwdMode: %d\n", ChangeFwdMode);
	cout("InsDomainMode: %d\n", InsDomainMode);
	cout("ConnectMethodOkMode: %d\n", ConnectMethodOkMode);
	cout("OKIPPrfxList: [%u]\n", getCount(OKIPPrfxList));
	cout("NGIPPrfxList: [%u]\n", getCount(NGIPPrfxList));
	cout("UR_Keys: [%u]\n", getCount(UR_Keys));
	cout("DR_Keys: [%u]\n", getCount(DR_Keys));
	cout("UploadFilterCommand: %s\n", UploadFilterCommand ? UploadFilterCommand : "<NONE>");
	cout("DownloadFilterCommand: %s\n", DownloadFilterCommand ? DownloadFilterCommand : "<NONE>");

	errorCase(IMAX / sizeof(Session_t) < ConnectMax);

	SessionList = na_(Session_t, ConnectMax);
	SessionStock = newList();

	{
		uint index;

		for (index = 0; index < ConnectMax; index++)
		{
			Session_t *i = SessionList + index;

			i->Sock = -1;
			i->FwdSock = -1;
			i->FwdName = strx("");
			i->Buff = newBlock();
			i->BkBuff = newBlock();
			addElement(SessionStock, (uint)i);
		}
	}

	return 0;
}
int main(int argc, char **argv)
{
	ConnectMax = 100;

	OKIPPrfxList = newList();
	NGIPPrfxList = newList();
	UR_Keys = newList();
	UR_Values = newList();
	UK_Keys = newList();
	DR_Keys = newList();
	DR_Values = newList();
	DK_Keys = newList();

	TunnelPerformTh = PerformTh;
	TunnelMain(ReadArgs, NULL, "HTTPProxy", NULL);
	TunnelPerformTh = NULL;
}
