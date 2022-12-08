#include "URL.h"

static char *ChgEnc_SJISToUTF8_x(char *str, int inverse)
{
	char *rFile = makeTempFile(NULL);
	char *wFile = makeTempFile(NULL);

	writeOneLineNoRet(rFile, str);
	memFree(str);

	if (inverse)
		UTF8ToSJISFile(rFile, wFile);
	else
		SJISToUTF8File(rFile, wFile);

	str = readText(wFile);
	removeFile(rFile);
	removeFile(wFile);
	memFree(rFile);
	memFree(wFile);
	return str;
}
static int IsNoPctChar(int chr) // ret: ? パーセントエンコーディングしない文字
{
	return
		m_isalpha(chr) ||
		m_isdecimal(chr) ||
		chr == '.' ||
		chr == ':' ||
		chr == '/' ||
		chr == '?' ||
		chr == '&' ||
		chr == '=';
}

char *urlEncoder(char *url)
{
	autoBlock_t *buff = newBlock();
	char *p;

	url = strx(url);
	url = ChgEnc_SJISToUTF8_x(url, 0);

	for (p = url; *p; p++)
	{
		if (IsNoPctChar(*p))
			addByte(buff, *p);
		else
			ab_addLine_x(buff, xcout("%%%02x", *p));
	}
	memFree(url);
	url = unbindBlock2Line(buff);
	return url;
}
char *urlDecoder(char *url)
{
	autoBlock_t *buff = newBlock();
	char *p;

	for (p = url; *p; p++)
	{
		if (*p == '%' && m_ishexadecimal(p[1]) && m_ishexadecimal(p[2]))
		{
			addByte(buff, m_c2i(p[1]) * 16 + m_c2i(p[2]));
			p += 2;
		}
		else
		{
			addByte(buff, *p);
		}
	}
	url = unbindBlock2Line(buff);
	url = ChgEnc_SJISToUTF8_x(url, 1);
	return url;
}

// c_
char *c_urlEncoder(char *url)
{
	static char *stock;
	memFree(stock);
	return stock = urlEncoder(url);
}
char *c_urlDecoder(char *url)
{
	static char *stock;
	memFree(stock);
	return stock = urlDecoder(url);
}

// _x
char *urlEncoder_x(char *url)
{
	char *ret = urlEncoder(url);
	memFree(url);
	return ret;
}
char *urlDecoder_x(char *url)
{
	char *ret = urlDecoder(url);
	memFree(url);
	return ret;
}
