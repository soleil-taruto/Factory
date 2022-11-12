#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\RingCipher2.h"
#include "C:\Factory\OpenSource\sha512.h"

#define ENCODE_PASSPHRASE "fjam " __DATE__ " " __TIME__
#define EXT_ENCODED "fjam"
#define EXT_MID "tmp_fjam"

static autoList_t *KeyTableList;
static int OutputAndDelete;

static void Fnlz(void)
{
	cphrReleaseKeyTableList(KeyTableList);
}
static void Init(void)
{
	autoBlock_t gab;

	sha512_makeHashLine(ENCODE_PASSPHRASE);
	KeyTableList = cphrCreateKeyTableList(gndBlockVar(sha512_hash, 64, gab), 32);
	addFinalizer(Fnlz);
}
static void PostGenWFile(char *rFile)
{
	if (OutputAndDelete)
//		semiRemovePath(rFile);
		removeFile(rFile);
}
static void DoEncode(char *rFile, char *wFile)
{
	cout("Encode\n");
	cout("< %s\n", rFile);
	cout("> %s\n", wFile);

	copyFile(rFile, wFile);
	rngcphrEncryptFile(wFile, KeyTableList);

	PostGenWFile(rFile);
}
static void DoDecode(char *rFile, char *wFile)
{
	char *mFile = addExt(strx(rFile), EXT_MID);

	cout("Decode\n");
	cout("< %s\n", rFile);
	cout("+ %s\n", mFile);
	cout("> %s\n", wFile);

	copyFile(rFile, mFile);
	rngcphrDecryptFile(mFile, KeyTableList);
	copyFile(mFile, wFile);

	removeFile_x(mFile);

	PostGenWFile(rFile);
}
static void DoEncode_NWF(char *rFile, char *wFile)
{
	if (!wFile)
	{
		wFile = addExt(strx(rFile), EXT_ENCODED);
		wFile = toCreatablePath(wFile, IMAX);

		DoEncode(rFile, wFile);

		memFree(wFile);
	}
	else
	{
		DoEncode(rFile, wFile);
	}
}
static void DoDecode_NWF(char *rFile, char *wFile)
{
	if (!wFile)
	{
		wFile = changeExt(rFile, "");
		wFile = toCreatablePath(wFile, IMAX);

		DoDecode(rFile, wFile);

		memFree(wFile);
	}
	else
	{
		DoDecode(rFile, wFile);
	}
}
static void Main2(void)
{
readArgs:
	if (argIs("/OAD"))
	{
		cout("+-------------------+\n");
		cout("| OUTPUT AND DELETE |\n");
		cout("+-------------------+\n");

		OutputAndDelete = 1;
		goto readArgs;
	}

	if (argIs("/E"))
	{
		char *rFile;
		char *wFile;

		if (hasArgs(2))
		{
			rFile = nextArg();
			wFile = nextArg();
		}
		else
		{
			rFile = hasArgs(1) ? nextArg() : c_dropFile();
			wFile = NULL;
		}
		DoEncode_NWF(rFile, wFile);
		return;
	}
	if (argIs("/D"))
	{
		char *rFile;
		char *wFile;

		if (hasArgs(2))
		{
			rFile = nextArg();
			wFile = nextArg();
		}
		else
		{
			rFile = hasArgs(1) ? nextArg() : c_dropFile();
			wFile = NULL;
		}
		DoDecode_NWF(rFile, wFile);
		return;
	}

	for (; ; )
	{
		char *rFile = c_dropFile();

		if (!_stricmp(getExt(rFile), EXT_ENCODED))
			DoDecode_NWF(rFile, NULL);
		else
			DoEncode_NWF(rFile, NULL);

		cout("\n");
	}
}
int main(int argc, char **argv)
{
	Init();
	Main2();
	termination(0);
}
