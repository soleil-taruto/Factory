#include "ZCluster.h"

// ---- cluster ----

int ZC_WithoutInfo;

static char *GetClusterExeFile(void)
{
	static char *file;

	if (!file)
		file = GetCollaboFile("C:\\Factory\\Tools\\Cluster.exe");

	return file;
}
static void DoCluster(char *file, char *dir, int mode) // mode: "MR"
{
	errorCase(m_isEmpty(file));
	errorCase(m_isEmpty(dir));

	// file == dir の場合この下辺りで error();

	if (mode == 'M')
	{
		cout("< %s\n", dir);
		cout("> %s\n", file);

		errorCase_m(existPath(file), "出力ファイルは既に存在します。@ ZC_DoCluster");
		errorCase(!existDir(dir));
	}
	else // R
	{
		cout("< %s\n", file);
		cout("> %s\n", dir);

		errorCase(!existFile(file));
		errorCase_m(existPath(dir), "出力ディレクトリは既に存在します。@ ZC_DoCluster");
	}
	coExecute_x(xcout("start \"\" /b /wait \"%s\" %s/%c \"%s\" \"%s\"", GetClusterExeFile(), ZC_WithoutInfo || mode == 'R' ? "" : "/I ", mode, file, dir));

	if (mode == 'M')
	{
		errorCase(!existFile(file));
	}
	else // R
	{
		errorCase(!existDir(dir));
	}
}
void ZC_Cluster(char *rDir, char *wFile)
{
	DoCluster(wFile, rDir, 'M');
}
void ZC_Uncluster(char *rFile, char *wDir)
{
	DoCluster(rFile, wDir, 'R');
}

// ---- pack ----

static char *GetPackExeFile(void)
{
	static char *file;

	if (!file)
		file = GetCollaboFile("C:\\app\\Kit\\Compress\\Compress.exe");

	return file;
}
static void DoPack(char *rFile, char *wFile, int mode) // mode: "CD"
{
	errorCase(m_isEmpty(rFile));
	errorCase(m_isEmpty(wFile));

	cout("< %s\n", rFile);
	cout("> %s\n", wFile);

	// rFile == wFile の場合この下辺りで error();

	errorCase(!existFile(rFile));
	errorCase_m(existPath(wFile), "出力ファイルは既に存在します。@ ZC_DoPack");

	coExecute_x(xcout("start \"\" /b /wait \"%s\" /%c \"%s\" \"%s\"", GetPackExeFile(), mode, rFile, wFile));

	errorCase(!existFile(wFile));
}
void ZC_Pack(char *rFile, char *wFile)
{
	DoPack(rFile, wFile, 'C');
}
void ZC_Unpack(char *rFile, char *wFile)
{
	DoPack(rFile, wFile, 'D');
}

// ---- encrypt ----

static int DoEncrypt(char *file, autoBlock_t *rawKey, int mode) // mode: "ED"
{
	autoList_t *keyTableList;
	int retval;

	errorCase(m_isEmpty(file));
	errorCase(!existFile(file));
	errorCase(!rawKey);

	keyTableList = rngcphrCreateKeyTableList(rawKey);

	if (mode == 'E')
	{
		rngcphrEncryptFile(file, keyTableList);
		retval = 1;
	}
	else // D
	{
		retval = rngcphrDecryptFile(file, keyTableList);
	}
	cphrReleaseKeyTableList(keyTableList);
cout("********** retval: %d\n", retval); // test test test test test
	return retval;
}
void ZC_Encrypt(char *file, autoBlock_t *rawKey)
{
	errorCase(!DoEncrypt(file, rawKey, 'E'));
}
int ZC_Decrypt(char *file, autoBlock_t *rawKey) // ret: ? 成功
{
	return DoEncrypt(file, rawKey, 'D');
}

// ----
