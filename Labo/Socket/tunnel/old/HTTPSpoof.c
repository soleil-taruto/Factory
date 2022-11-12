/*
	HTTPSpoof.exe RECV-PORT DMY-FWD-HOST DMY-FWD-PORT /S

		/S ... ��~����B

	HTTPSpoof.exe RECV-PORT DMY-FWD-HOST DMY-FWD-PORT [/D] [/R H_KEY H_VALUE]...

		/D ... ���N�G�X�gURL�Ƀh���C������}������B
		/R ... �u���������͒ǉ�����w�b�_�t�B�[���h

	----
	���[�J���ւ̃A�N�Z�X���v���L�V��ʂ��ĊO���ɔ�΂��B

		_http://localhost/xxx �� inet.com:8080 �ɔ�΂������B

			tunnel 80 inet.com 8080

		�X�Ƀv���L�V proxy:3210 ��ʂ������B

			HTTPSpoof 80 proxy 3210 /d /r Host inet.com:8080

	----
	�����_

		Connection: Keep-Alive ���Ή�
		����̃��b�Z�[�W�ɂ��ĉ����ł��Ȃ��B
*/

#include "..\libs\Tunnel.h"
#include "..\libs\HTTPParse.h"
#include "C:\Factory\Common\Options\SockStream.h"

static int PutDomainUrl;
static autoList_t *R_Keys;
static autoList_t *R_Values;

static void SpoofHeader(void)
{
	// �w�b�_�t�B�[���h�u���E�ǉ�
	{
		char *r_key;
		uint r_ndx;

		foreach (R_Keys, r_key, r_ndx)
		{
			uint h_ndx = findLineCase(HttpDat.H_Keys, r_key, 1);
			char *h_key;
			char *h_val;
			char *r_val = getLine(R_Values, r_ndx);

			if (h_ndx == getCount(HttpDat.H_Keys))
			{
				addElement(HttpDat.H_Keys, (uint)strx(r_key));
				addElement(HttpDat.H_Values, (uint)strx("[����`]"));
			}
			h_key = getLine(HttpDat.H_Keys, h_ndx);
			h_val = getLine(HttpDat.H_Values, h_ndx);

			cout("H_K: [%s]\n", h_key);
			cout("H_V: [%s] -> [%s]\n", h_val, r_val);

			memFree(h_val);
			setElement(HttpDat.H_Values, h_ndx, (uint)strx(r_val));
		}
	}

	if (PutDomainUrl) // �h���C������url�ɑ}��
	{
		char *host = refLine(HttpDat.H_Values, findLineCase(HttpDat.H_Keys, "Host", 1));
		char *insPtn;

		cout("PutDmnUrl\n");
		cout("host: %s\n", host);
		cout("< %s\n", HttpDat.H_Request);

		insPtn = xcout("GET http://%s/", host);

		if (5 <= strlen(HttpDat.H_Request))
			eraseLine(HttpDat.H_Request, 5); // -= "GET /"

		HttpDat.H_Request = insertLine_x(HttpDat.H_Request, 0, insPtn);

		cout("> %s\n", HttpDat.H_Request);
	}
}
static char *HFldFolding(char *line)
{
	uint index = 70; // ����Ȓ����L�[�͖������낤...

	while (index + 10 <= strlen(line))
	{
		if (line[index] <= '\x20') // ? WSP
		{
			line = insertLine(line, index, "\r\n");
			line[index + 2] = '\t';
			index += 50;
		}
		index++;
	}
	return line;
}
static void RemakeHeader(autoBlock_t *buff)
{
	uint index;

	setSize(buff, 0);
	ab_addLine(buff, HttpDat.H_Request);
	ab_addLine(buff, "\r\n");

	for (index = 0; index < getCount(HttpDat.H_Keys); index++)
	{
		char *key = getLine(HttpDat.H_Keys, index);
		char *value = getLine(HttpDat.H_Values, index);
		char *line;

		line = xcout("%s: %s", key, value);
		line = HFldFolding(line);

		ab_addLine_x(buff, line);
		ab_addLine(buff, "\r\n");
	}
	ab_addLine(buff, "\r\n");
}

static int DoConnect(void) // ret: -1 == �ڑ��ł��Ȃ������B
{
	static uchar ip[4];
	return sockConnect(ip, FwdHost, FwdPortNo);
}
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
static void PerformTh(int sock, char *strip)
{
	SockStream_t *ss = CreateSockStream(sock, 60);
	autoBlock_t *buff = newBlock();

	cout("�ڑ� %d\n", sock);

	for (; ; )
	{
		int chr = SockRecvChar(ss);
		uint endPos;

		if (chr == EOF)
			break;

		addByte(buff, chr);
		endPos = getSize(buff);

		if (4 <= endPos && !memcmp((uchar *)directGetBuffer(buff) + endPos - 4, "\r\n\r\n", 4))
		{
			int fwdSock;

			HTTPParse(buff);
			SpoofHeader();
			RemakeHeader(buff);

			fwdSock = DoConnect();

			cout("FWD_SOCK: %d\n", fwdSock);

			if (fwdSock != -1)
			{
				autoBlock_t *buffTmp = buff; // PreDataFltr() �� NULL ���Z�b�g�����B

				ab_addBytes_x(buff, SockRipRecvBuffer(ss));

				CrossChannel(sock, fwdSock, PreDataFltr, (uint)&buffTmp, NULL, 0);
				sockDisconnect(fwdSock);
			}
			break;
		}
		if (65000 < endPos)
		{
			cout("OVERFLOW_HEADER_SIZE\n");
			break;
		}
	}
	ReleaseSockStream(ss);
	releaseAutoBlock(buff);

	cout("�ؒf %d\n", sock);
}
static int ReadArgs(void)
{
	if (argIs("/D"))
	{
		PutDomainUrl = 1;
		return 1;
	}
	if (argIs("/R"))
	{
		addElement(R_Keys, (uint)nextArg());
		addElement(R_Values, (uint)nextArg());
		return 1;
	}

	// �K�{
	addElement(R_Keys, (uint)"Connection");
	addElement(R_Values, (uint)"close");

	cout("PutDomainUrl: %d\n", PutDomainUrl);
	cout("R_Keys: %u\n", getCount(R_Keys));

	return 0;
}
int main(int argc, char **argv)
{
	ConnectMax = 30;

	R_Keys = newList();
	R_Values = newList();

	TunnelPerformTh = PerformTh;
	TunnelMain(ReadArgs, NULL, "HTTPSpoof", NULL);
	TunnelPerformTh = NULL;
}
