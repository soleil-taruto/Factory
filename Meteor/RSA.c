#include "RSA.h"

#if 0

#define KEY_SIZE      3072
#define PLAIN_SIZEMIN    0
#define PLAIN_SIZEMAX  303 // RSA_plain_max == 343, 343 以下で最大の 16 の倍数 -> 336, 336 - 16_hash - 16_randPart - 1_padding_min -> 303
#define CIPHER_SIZE    384
#define MODULUS_SIZE   384
#define EXPONENT_SIZE    3
#define P_SIZE         192
#define Q_SIZE         192
#define DP_SIZE        192
#define DQ_SIZE        192
#define INVERSE_Q_SIZE 192
#define D_SIZE         384

#elif 1

#define KEY_SIZE      2048
#define PLAIN_SIZEMIN    0
#define PLAIN_SIZEMAX  175 // RSA_plain_max == 215, 215 以下で最大の 16 の倍数 -> 208, 208 - 16_hash - 16_randPart - 1_padding_min -> 175
#define CIPHER_SIZE    256
#define MODULUS_SIZE   256
#define EXPONENT_SIZE    3
#define P_SIZE         128
#define Q_SIZE         128
#define DP_SIZE        128
#define DQ_SIZE        128
#define INVERSE_Q_SIZE 128
#define D_SIZE         256

#else

#define KEY_SIZE      1024
#define PLAIN_SIZEMIN    0
#define PLAIN_SIZEMAX   87 // RSA_plain_max == 87, 87 以下で最大の 16 の倍数 -> 80, 80 - 16_hash - 16_randPart - 1_padding_min -> 47
#define CIPHER_SIZE    128
#define MODULUS_SIZE   128
#define EXPONENT_SIZE    3
#define P_SIZE          64
#define Q_SIZE          64
#define DP_SIZE         64
#define DQ_SIZE         64
#define INVERSE_Q_SIZE  64
#define D_SIZE         128

#endif

uint RSA_PlainSizeMax = PLAIN_SIZEMAX;

static char *GetRSAExeFile(void)
{
	static char *file;

	if (!file)
//		file = GetCollaboFile("C:\\app\\Kit\\RSA\\RSA.exe"); // 廃止 @ 2023.4.13
		file = GetCollaboFile("C:\\Factory\\tmp\\Kit_Dummy\\RSA\\RSA.exe");

	return file;
}

// ---- key file ----

static XNode_t *KRoot;

static char *ReadXText(char *name) // ret: xml_text
{
	XNode_t *node;
	uint index;

	foreach (KRoot->Children, node, index)
		if (!strcmp(node->Name, name))
			return node->Text;

	error();
	return NULL; // dummy
}
static void AddXText(autoBlock_t *buff, char *name, autoBlock_t *block) // block: kd_bytes
{
	autoBlock_t *element = encodeBase64(block);

	addByte(buff, '<');
	ab_addLine(buff, name);
	addByte(buff, '>');

	ab_addBytes(buff, element);

	addByte(buff, '<');
	addByte(buff, '/');
	ab_addLine(buff, name);
	addByte(buff, '>');

	releaseAutoBlock(element);
}
static void AddXText_cx(autoBlock_t *buff, char *name, autoBlock_t *block)
{
	AddXText(buff, name, block);
	releaseAutoBlock(block);
}

static autoBlock_t *KData;
static uint KPos;

static void AddKText(char *text, uint assumedSize) // text: xml_text
{
	autoBlock_t gab_text;
	autoBlock_t *element;

	gab_text = gndBlockLine(text);
	element = decodeBase64(&gab_text);

	cout("KEY_ELEMENT_SIZE: %u (ASSUMED: %u)\n", getSize(element), assumedSize);

	errorCase_m(getSize(element) != assumedSize, "RXKF_想定外のキー要素サイズ");

	addBytes(KData, element);
	releaseAutoBlock(element);
}
static autoBlock_t *ReadKText(uint size) // ret: kd_bytes
{
	autoBlock_t *block = NULL;

	errorCase(getSize(KData) < KPos + size);

	block = ab_makeSubBytes(KData, KPos, size);
	KPos += size;
	return block;
}

static autoBlock_t *ReadXMLKeyFile(char *file, int privateMode) // file の破損は想定しない。
{
	char *modulus;
	char *exponent;
	char *p;
	char *q;
	char *dP;
	char *dQ;
	char *inverseQ;
	char *d;

	errorCase(m_isEmpty(file));
	errorCase(!existFile(file));
	// privateMode

	KRoot = readXMLFile(file);

	modulus  = ReadXText("Modulus");
	exponent = ReadXText("Exponent");

	if (privateMode)
	{
		p        = ReadXText("P");
		q        = ReadXText("Q");
		dP       = ReadXText("DP");
		dQ       = ReadXText("DQ");
		inverseQ = ReadXText("InverseQ");
		d        = ReadXText("D");
	}
	KData = newBlock();
//	KPos = 0;

	AddKText(modulus,  MODULUS_SIZE);
	AddKText(exponent, EXPONENT_SIZE);

	if (privateMode)
	{
		AddKText(p,        P_SIZE);
		AddKText(q,        Q_SIZE);
		AddKText(dP,       DP_SIZE);
		AddKText(dQ,       DQ_SIZE);
		AddKText(inverseQ, INVERSE_Q_SIZE);
		AddKText(d,        D_SIZE);
	}

	cout("KEY_SIZE: %u\n", getSize(KData));

	ReleaseXNode(KRoot);
	return KData;
}
static void WriteXMLKeyFile(char *file, int privateMode, autoBlock_t *keyData) // keyData が破損している場合、適当な内容を file に書き出す。
{
	uint assumedKeySize;
	autoBlock_t *modulus;
	autoBlock_t *exponent;
	autoBlock_t *p;
	autoBlock_t *q;
	autoBlock_t *dP;
	autoBlock_t *dQ;
	autoBlock_t *inverseQ;
	autoBlock_t *d;
	autoBlock_t *buff;

	errorCase(m_isEmpty(file));
	// privateMode
	errorCase(!keyData);

	removePathIfExist(file);

	KData = keyData;
	KPos = 0;

	assumedKeySize = 0;
	assumedKeySize += MODULUS_SIZE;
	assumedKeySize += EXPONENT_SIZE;

	if (privateMode)
	{
		assumedKeySize += P_SIZE;
		assumedKeySize += Q_SIZE;
		assumedKeySize += DP_SIZE;
		assumedKeySize += DQ_SIZE;
		assumedKeySize += INVERSE_Q_SIZE;
		assumedKeySize += D_SIZE;
	}
	if (getSize(KData) != assumedKeySize)
	{
		cout("WXKF_想定外のキーデータサイズ %u (ASSUMED: %u)\n", getSize(KData), assumedKeySize);
		setSize(KData, assumedKeySize);
	}

	modulus  = ReadKText(MODULUS_SIZE);
	exponent = ReadKText(EXPONENT_SIZE);

	if (privateMode)
	{
		p        = ReadKText(P_SIZE);
		q        = ReadKText(Q_SIZE);
		dP       = ReadKText(DP_SIZE);
		dQ       = ReadKText(DQ_SIZE);
		inverseQ = ReadKText(INVERSE_Q_SIZE);
		d        = ReadKText(D_SIZE);
	}

	buff = newBlock();
	ab_addLine(buff, "<RSAKeyValue>");

	AddXText_cx(buff, "Modulus",  modulus);
	AddXText_cx(buff, "Exponent", exponent);

	if (privateMode)
	{
		AddXText_cx(buff, "P",        p);
		AddXText_cx(buff, "Q",        q);
		AddXText_cx(buff, "DP",       dP);
		AddXText_cx(buff, "DQ",       dQ);
		AddXText_cx(buff, "InverseQ", inverseQ);
		AddXText_cx(buff, "D",        d);
	}
	ab_addLine(buff, "</RSAKeyValue>");
	writeBinary(file, buff);
	releaseAutoBlock(buff);
}

static autoBlock_t *ReadXMLKeyFile_Check(char *file, int privateMode)
{
	char *testXKFile = makeTempFile("test_xk");

	ReadXMLKeyFile(file, privateMode);
	WriteXMLKeyFile(testXKFile, privateMode, KData);

	errorCase_m(!isSameFile(file, testXKFile), "RXKF_想定外のXMLキーファイルフォーマット");

	removeFile(testXKFile);
	memFree(testXKFile);

	return KData;
}

// --- jamming ----

static autoList_t *GetJamKeyTableList(void) // ret: bind
{
	static autoList_t *keyTableList;

	if (!keyTableList)
	{
		keyTableList = newList();

		addElement(keyTableList, (uint)camellia_createKeyTable(GetConcreteRawKey()));
		addElement(keyTableList, (uint)camellia_createKeyTable(GetConcreteRawKey()));
	}
	return keyTableList;
}
static void JamEncrypt(autoBlock_t *data)
{
	// += Padding
	{
		uint size = (~getSize(data) & 0x0f) + 1;
		uint index;

		for (index = 0; index < size; index++)
			addByte(data, size);
	}
	// += RandPart
	{
		autoBlock_t *randPart = makeCryptoRandBlock(16);

		ab_addBytes(data, randPart);
		releaseAutoBlock(randPart);
	}
	// += Hash
	{
		autoBlock_t *hash = md5_makeHashBlock(data);

		ab_addBytes(data, hash);
		releaseAutoBlock(hash);
	}
	// Encrypt
	{
		rngcphrEncryptBlock(data, GetJamKeyTableList());
	}
}
static int JamDecrypt(autoBlock_t *data) // ret: ? 成功
{
	// Decrypt
	{
		if (
			getSize(data) < 32 ||
			getSize(data) % 16 != 0
			)
			return 0;

		rngcphrDecryptBlock(data, GetJamKeyTableList());
	}
	// -= Hash
	{
		autoBlock_t *r_hash;
		autoBlock_t *m_hash;
		int retval;

		if (getSize(data) < 16)
			return 0;

		r_hash = unaddBytes(data, 16);
		m_hash = md5_makeHashBlock(data);

		retval = isSameBlock(r_hash, m_hash);

		releaseAutoBlock(r_hash);
		releaseAutoBlock(m_hash);

		if (!retval)
			return 0;
	}
	// -= RandPart
	{
		if (getSize(data) < 16)
			return 0;

		setSize(data, getSize(data) - 16);
	}
	// -= Padding
	{
		uint size;

		if (getSize(data) < 1)
			return 0;

		size = getByte(data, getSize(data) - 1);

		if (getSize(data) < size)
			return 0;

		setSize(data, getSize(data) - size);
	}
	return 1;
}

// ----

static autoBlock_t *S_PublicKey;
static autoBlock_t *S_PrivateKey;

void RSA_GenerateKey(void)
{
	char *publicKeyFile  = makeTempPath("pub");
	char *privateKeyFile = makeTempPath("prv");

	if (S_PublicKey)
	{
		releaseAutoBlock(S_PublicKey);
		releaseAutoBlock(S_PrivateKey);
	}

	execute_x(xcout("start \"\" /b /wait \"%s\" /G %u \"%s\" \"%s\"", GetRSAExeFile(), KEY_SIZE, publicKeyFile, privateKeyFile));

	// publicKeyFile, privateKeyFile の有無のチェック -> ReadXMLKeyFile()

	S_PublicKey  = ReadXMLKeyFile_Check(publicKeyFile, 0);
	S_PrivateKey = ReadXMLKeyFile_Check(privateKeyFile, 1);

	removeFile(publicKeyFile);
	removeFile(privateKeyFile);

	memFree(publicKeyFile);
	memFree(privateKeyFile);
}
autoBlock_t *RSA_GetPublicKey(void) // ret: 公開鍵のコピー
{
	errorCase(!S_PublicKey);

	return copyAutoBlock(S_PublicKey);
}
/*
	publicKey:
		公開鍵
		ネットワーク越しに受け取った（であろう）データなので破損していても良い。
		破損していた場合、適当な暗号データを返す。
*/
autoBlock_t *RSA_Encrypt(autoBlock_t *plainData, autoBlock_t *publicKey) // ret: NULL は返さない。
{
	char *publicKeyFile = makeTempPath(NULL);
	char *plainDataFile = makeTempPath(NULL);
	char *cipherDataFile = makeTempPath(NULL);
	autoBlock_t *cipherData;

	errorCase(!plainData);
	errorCase(!publicKey);
	errorCase(!m_isRange(getSize(plainData), PLAIN_SIZEMIN, PLAIN_SIZEMAX));

	plainData = copyAutoBlock(plainData);
	JamEncrypt(plainData);

	WriteXMLKeyFile(publicKeyFile, 0, publicKey);
	writeBinary(plainDataFile, plainData);

	releaseAutoBlock(plainData);

	execute_x(xcout("start \"\" /b /wait \"%s\" /E \"%s\" \"%s\" \"%s\"", GetRSAExeFile(), publicKeyFile, plainDataFile, cipherDataFile));

	if (!existFile(cipherDataFile))
	{
		cout("RSA_E_暗号化に失敗しました。(暗号化実行失敗)\n");
		createFile(cipherDataFile);
	}
	cipherData = readBinary(cipherDataFile);

	if (getSize(cipherData) != CIPHER_SIZE)
	{
		cout("RSA_E_暗号化に失敗しました。(想定外の暗号データサイズ)\n");
		setSize(cipherData, CIPHER_SIZE);
	}
	removeFile(publicKeyFile);
	removeFile(plainDataFile);
	removeFile(cipherDataFile);

	memFree(publicKeyFile);
	memFree(plainDataFile);
	memFree(cipherDataFile);

	return cipherData;
}
autoBlock_t *RSA_Decrypt(autoBlock_t *cipherData) // ret: NULL == 復号に失敗した。
{
	char *privateKeyFile = makeTempPath(NULL);
	char *cipherDataFile = makeTempPath(NULL);
	char *plainDataFile = makeTempPath(NULL);
	autoBlock_t *plainData = NULL;

	errorCase(!cipherData);
	errorCase(!S_PrivateKey);

	if (getSize(cipherData) != CIPHER_SIZE)
	{
		cout("RSA_D_想定外の暗号データサイズ %u (ASSUMED: %u)\n", getSize(cipherData), CIPHER_SIZE);
		goto endFunc;
	}
	WriteXMLKeyFile(privateKeyFile, 1, S_PrivateKey);
	writeBinary(cipherDataFile, cipherData);

	execute_x(xcout("start \"\" /b /wait \"%s\" /D \"%s\" \"%s\" \"%s\"", GetRSAExeFile(), privateKeyFile, cipherDataFile, plainDataFile));

	if (!existFile(plainDataFile))
	{
		cout("RSA_D_復号に失敗しました。(復号実行失敗)\n");
		goto endFunc;
	}
	plainData = readBinary(plainDataFile);

	if (!JamDecrypt(plainData))
	{
		cout("RSA_D_復号に失敗しました。(平文データの破損)\n");
		releaseAutoBlock(plainData);
		plainData = NULL;
		goto endFunc;
	}

endFunc:
	removeFileIfExist(privateKeyFile);
	removeFileIfExist(cipherDataFile);
	removeFileIfExist(plainDataFile);

	memFree(privateKeyFile);
	memFree(cipherDataFile);
	memFree(plainDataFile);

	return plainData;
}
