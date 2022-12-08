#include "Client.h"

autoList_t *httpExtraHeader;
uint httpBlockTimeout; // ���ʐM�^�C���A�E�g, �b, 0 == ������

static char *Domain;
static uint Portno;
static char *ProxyDomain;
static uint ProxyPortno;
static char *Path;
static uint Timeout; // �ڑ����Ă���̃^�C���A�E�g, �b, 0 == ������
static autoBlock_t *Content;
static uint RetContentSizeMax;
static autoBlock_t *RetContent;
static char *RetContFile;

static char *GetBoundary(void)
{
	uint index;

	for (index = 2; index < getSize(Content); index++) // �ŏ��̂Q������ "--"
	{
		int chr;

		if (100 < index) // boundary�������70�����ȉ�
			break;

		chr = getByte(Content, index);

		if (chr == '\r')
			break;
	}
	return unbindBlock2Line(getSubBytes(Content, 2, index - 2));
}
static void Perform(int sock)
{
	SockStream_t *ss = CreateSockStream(sock, Timeout);
	char *header;
	uchar *content;
	uint index;
	uint csize;
	int ctflag = 0;

	SetSockStreamBlockTimeout(ss, httpBlockTimeout);

	// 1�s��

	header = strx("");
	header = addLine(header, Content ? "POST" : "GET");
	header = addChar(header, ' ');

	if (ProxyDomain)
	{
		header = addLine(header, "http://");
		header = addLine(header, Domain);

		if (Portno != 80)
			header = addLine_x(header, xcout(":%u", Portno));
	}
	header = addLine(header, Path);
	header = addLine(header, " HTTP/1.1");

	SockSendLine(ss, header);
	memFree(header);

	// Host�t�B�[���h

	header = strx("");
	header = addLine(header, "Host: ");
	header = addLine(header, Domain);

	if (Portno != 80)
		header = addLine_x(header, xcout(":%u", Portno));

	SockSendLine(ss, header);
	memFree(header);

	// �g���w�b�_�t�B�[���h

	if (httpExtraHeader)
	{
		foreach (httpExtraHeader, header, index)
		{
			if (startsWithICase(header, "Content-Type:"))
				ctflag = 1;

			SockSendLine(ss, header);
		}
	}

	// ContentType�t�B�[���h

	if (Content && getSize(Content) && !ctflag)
	{
		int chr = getByte(Content, 0);

		if (chr == '-')
		{
			char *boundary = GetBoundary();

			SockSendLine_x(ss, xcout("Content-Type: multipart/form-data; boundary=\"%s\"", boundary));
			memFree(boundary);
		}
		else if (chr == '{')
		{
			SockSendLine(ss, "Content-Type: application/json");
		}
		else
		{
//			SockSendLine(ss, "Content-Type: application/octet-stream");
		}
	}

	// ContentLength�t�B�[���h + Content

	if (Content)
	{
		SockSendLine_x(ss, xcout("Content-Length: %u", getSize(Content)));
		SockSendLine(ss, "");
		SockSendBlock(ss, directGetBuffer(Content), getSize(Content));
	}
	else
	{
		SockSendLine(ss, "");
	}

	SockFlush(ss);

	// ��M

	httpRecvRequestFirstWaitDisable = 1;

	/*
		httpRecvRequest*()�����X�|���X�̎�M�Ɏg���B
	*/
	if (RetContFile)
	{
		httpRecvRequestFile(ss, &header, RetContFile); // RetContFile �͕K���쐬�����B(���s���͋�)

		content = (uchar *)strx("Dummy");
		csize = strlen((char *)content) + 1;
	}
	else
	{
		httpRecvRequestMax(ss, &header, &content, &csize, RetContentSizeMax);
	}

	httpRecvRequestFirstWaitDisable = 0;
	memFree(header);

	RetContent = bindBlock(content, csize);

	if (IsEOFSockStream(ss) && !ss->Extra.M4UServerMode_Operated)
	{
		releaseAutoBlock(RetContent);
		RetContent = NULL;
	}
	ReleaseSockStream(ss);
}

/*
	http�N���C�A���g�Ƃ��ĎI�Ƀ��N�G�X�g�𑗂�A���X�|���X���󂯎��B

	domain - �I�̃h���C���� (""�̂Ƃ�����localhost)
	portno - �I�̃|�[�g�ԍ� (1-65535)
	proxyDomain - �v���L�V�I�̃h���C����           ����ʂ��Ȃ��ꍇ�� NULL ���w�肷�邱�ƁB(""�̂Ƃ�����localhost)
	proxyPortno - �v���L�V�I�̃|�[�g�ԍ� (1-65535) ����ʂ��Ȃ��ꍇ�͉��ł��ǂ��B
	path - ���N�G�X�g�̃p�X�BURL�̃h���C�����̌��ɘA�����邾���Ȃ̂ŁA�N�G��������ꍇ�͌��ɘA������B(�p�X�̐擪�� "/" ��Y�ꂸ�ɁA���Ƃ��� "/index.html")
	content - NULL �̂Ƃ� GET �����łȂ��Ƃ� content �� CONTENT (�}���`�p�[�g) �Ƃ��� POST ���N�G�X�g�𓊂���B
	timeout - �����^�C���A�E�g (�~���b) 0: ������
	sizemax - ��M�f�[�^�ő�T�C�Y (�o�C�g)

	��M�������X�|���X�̃f�[�^����Ԃ��B
	�f�[�^������̏ꍇ�A0 �o�C�g�̃u���b�N��Ԃ��B�ڑ��E�ʐM�Ɏ��s����� NULL ��Ԃ��B
*/
autoBlock_t *httpSendRequest(char *domain, uint portno, char *proxyDomain, uint proxyPortno, char *path, autoBlock_t *content, uint timeout, uint sizemax)
{
	uchar ip[4];

	errorCase(!domain);
	errorCase(!portno || 0xffff < portno);
	errorCase(proxyDomain && (!proxyPortno || 0xffff < proxyPortno));
	errorCase(!path);

	Domain = domain;
	Portno = portno;
	ProxyDomain = proxyDomain;
	ProxyPortno = proxyPortno;
	Path = path;
	Timeout = timeout;
	Content = content;
	RetContentSizeMax = sizemax;
	RetContent = NULL;

	if (proxyDomain)
	{
		domain = proxyDomain;
		portno = proxyPortno;
	}

	*(uint *)ip = 0;
	sockClientUserTransmit(ip, domain, portno, Perform);

	return RetContent;
}
/*
	�߂�l: �����������ǂ����B

	retContFile:
		���������ꍇ�A��M���� CONTENT ���o�͂���B
		���s�����ꍇ�A�폜����B

	��M�t�@�C���̍ő�T�C�Y�� httpMultiPartContentLenMax �ɐݒ肷�邱�ƁB
*/
int httpSendRequestFile(char *domain, uint portno, char *proxyDomain, uint proxyPortno, char *path, autoBlock_t *content, uint timeout, char *retContFile)
{
	autoBlock_t *dmyCont;

	errorCase(!retContFile);

	RetContFile = retContFile;
	dmyCont = httpSendRequest(domain, portno, proxyDomain, proxyPortno, path, content, timeout, 0);
	RetContFile = NULL;

	if (dmyCont)
	{
		releaseAutoBlock(dmyCont);
		return 1;
	}
	removeFileIfExist(retContFile);
	return 0;
}
