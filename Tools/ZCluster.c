/*
	ZCluster.exe [/KB KEY-BUNDLE] [/OAD] (/M 入力パス 出力クラスタファイル |
		/MO 入力パス 出力クラスタファイル |
		/BM 入力パス 出力クラスタファイル |
		/RF 入力クラスタファイル 出力ファイル |
		/R 入力クラスタファイル 出力ディレクトリ |
		/P 入力ファイル 出力ファイル |
		/U 入力ファイル 出力ファイル |
		/E 入力ファイル 出力ファイル |
		/D 入力ファイル 出力ファイル |
		/E+ 入出力ファイル |
		/D+ 入出力ファイル)   ★コマンドモード

	ZCluster.exe [/KB KEY-BUNDLE] [/C] [/1] [/-OW] [/OAD] [入力パス | 入出力パス]   ★自動判定モード

	KEY-BUNDLE ... 鍵束ファイル or * or *PASS
	/C   ... 自動判定モードのリストア時にカレントディレクトリに出力する。
	/1   ... 自動判定モードのクラスタ作成時にフリーディレクトリに出力する。
	/-OW ... 自動判定モードで上書き禁止（上書きしようとするとエラー）
	/OAD ... 出力したら入力を削除する
	/M   ... クラスタファイル作成
	/MO  ... クラスタファイル作成（上書きする）
	/BM  ... クラスタファイル作成（上書き＋１世代だけバックアップする）
	/R   ... リストア
	/P   ... 圧縮
	/U   ... 展開
	/E   ... 暗号化（KEY-BUNDLE 必須）
	/D   ... 復号  （KEY-BUNDLE 必須）
	/E+  ... 暗号化（KEY-BUNDLE 必須）
	/D+  ... 復号  （KEY-BUNDLE 必須）
	/-I  ... クラスタファイルを生成するときファイル属性とタイムスタンプを保存しない。

	★コマンドモード ... 常に上書きモード
	★自動判定モード ... デフォルトで上書きモード, /-OW 指定により上書き禁止（上書きしようとするとエラー）
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Meteor\ZCluster.h"

#define EXT_CLUSTER "clu"
#define EXT_PACKED "gz"
#define EXT_ENCRYPTED "enc"

static int OutputAndDeleteMode;

static void PostOutput(char *rPath)
{
	cout("POST_OUTPUT: %s\n", rPath);

	if (OutputAndDeleteMode)
	{
		cout("[DELETE]\n");

		recurRemovePath(rPath);
	}
}

// ---- auto action ----

static int RestoreCurrentDirMode;
static int MakeClusterFreeDirMode;
static int NoOverwriteMode;

static char *MakeRestorePath(char *path, int rcdm)
{
	if (!rcdm)
		return changeExt_xc(combine_xc(makeFreeDir(), getLocal(path)), "");

	path = changeExt(path, "");

	if (!NoOverwriteMode)
		recurRemovePathIfExist(path);

	return path;
}
static char *AA_Restore(char *rFile, autoBlock_t *rawKey, int rcdm)
{
	char *ext;

	if (!existFile(rFile))
		return NULL;

	ext = getExt(rFile);

	if (!_stricmp(ext, EXT_CLUSTER))
	{
		char *wDir = MakeRestorePath(rFile, rcdm);

		LOGPOS();
		ZC_Uncluster(rFile, wDir);
		LOGPOS();
		return wDir;
	}
	if (!_stricmp(ext, EXT_PACKED))
	{
		char *wFile = MakeRestorePath(rFile, rcdm);

		LOGPOS();
		ZC_Unpack(rFile, wFile);
		LOGPOS();
		return wFile;
	}
	if (!_stricmp(ext, EXT_ENCRYPTED))
	{
		char *wFile;

		errorCase_m(!rawKey, "キーハンドルがセットされていません。@ AA_R");

		wFile = MakeRestorePath(rFile, rcdm);
		copyFile(rFile, wFile);
		LOGPOS();
		errorCase_m(!ZC_Decrypt(wFile, rawKey), "復号に失敗しました。@ AA_R");
		LOGPOS();
		return wFile;
	}
	return NULL;
}
static void AA_PackAndEncrypt(char *rFile, autoBlock_t *rawKey, int mcfdm)
{
	char *wFile;

	if (mcfdm)
		wFile = combine_xc(makeFreeDir(), getLocal(rFile));
	else
		wFile = strx(rFile);

	wFile = addExt(wFile, EXT_PACKED);

	if (existPath(wFile))
	{
		errorCase_m(NoOverwriteMode, "上書きしようとしました。@ ZC_P_AA_PAE");
		recurRemovePath(wFile);
	}
	LOGPOS();
	ZC_Pack(rFile, wFile);
	LOGPOS();

	if (rawKey)
	{
		char *tmp = strx(wFile);

		wFile = addExt(wFile, EXT_ENCRYPTED);

		if (existPath(wFile))
		{
			errorCase_m(NoOverwriteMode, "上書きしようとしました。@ ZC_E_AA_PAE");
			recurRemovePath(wFile);
		}
		moveFile(tmp, wFile);
		memFree(tmp);

		LOGPOS();
		ZC_Encrypt(wFile, rawKey);
		LOGPOS();
	}
	if (MakeClusterFreeDirMode)
	{
		char *wDir = getParent(wFile);

		execute_x(xcout("START \"\" \"%s\"", wDir));
		memFree(wDir);
	}
	memFree(wFile);
}
static void AutoAction(char *rPath, autoBlock_t *rawKey)
{
	errorCase(m_isEmpty(rPath));
	errorCase(!existPath(rPath));
	// rawKey

	rPath = makeFullPath(rPath);

	if (existFile(rPath))
	{
		char *ext = getExt(rPath);

		if (
			!_stricmp(ext, EXT_CLUSTER) ||
			!_stricmp(ext, EXT_PACKED) ||
			!_stricmp(ext, EXT_ENCRYPTED)
			)
			// 復元モード
		{
			char *wPath = AA_Restore(rPath, rawKey, RestoreCurrentDirMode);

			errorCase(!wPath);

			for (; ; )
			{
				char *nextWPath = AA_Restore(wPath, rawKey, 1);

				if (!nextWPath)
					break;

				removeFile(wPath);
				memFree(wPath);
				wPath = nextWPath;
			}
			if (!RestoreCurrentDirMode)
			{
				char *wDir = getParent(wPath);

				execute_x(xcout("START \"\" \"%s\"", wDir));
				memFree(wDir);
			}
			memFree(wPath);
		}
		else // クラスタ化モード
		{
			AA_PackAndEncrypt(rPath, rawKey, MakeClusterFreeDirMode);
		}
	}
	else // rPath == DIR -> クラスタ化モード
	{
		char *midFile;

		if (MakeClusterFreeDirMode)
			midFile = combine_xc(makeFreeDir(), getLocal(rPath));
		else
			midFile = strx(rPath);

		midFile = addExt(midFile, EXT_CLUSTER);

		if (existPath(midFile))
		{
			errorCase_m(NoOverwriteMode, "上書きしようとしました。@ ZC_C_AA");
			recurRemovePath(midFile);
		}
		ZC_Cluster(rPath, midFile);
		AA_PackAndEncrypt(midFile, rawKey, 0);
		removeFile(midFile);
		memFree(midFile);
	}
	PostOutput(rPath);
	memFree(rPath);
}

// ----

static void MakeCluster(char *rPath, char *wFile, autoBlock_t *rawKey)
{
	errorCase(m_isEmpty(rPath));
	errorCase(m_isEmpty(wFile));
	errorCase(!existPath(rPath));
	errorCase(existPath(wFile)); // rPath == wFile の場合この辺で error();
	// rawKey

	if (existDir(rPath))
	{
		char *midFile = makeTempPath("zc_mid");

		LOGPOS();
		ZC_Cluster(rPath, midFile);
		LOGPOS();
		ZC_Pack(midFile, wFile);
		LOGPOS();

		removeFile(midFile);
		memFree(midFile);
	}
	else
	{
		LOGPOS();
		ZC_Pack(rPath, wFile);
		LOGPOS();
	}
	if (rawKey)
	{
		LOGPOS();
		ZC_Encrypt(wFile, rawKey);
		LOGPOS();
	}
	PostOutput(rPath);
}
static void Restore(char *rFile, char *wPath, autoBlock_t *rawKey, int restoreDirMode)
{
	char *midFile;

	errorCase(m_isEmpty(rFile));
	errorCase(m_isEmpty(wPath));
	errorCase(!existFile(rFile));
	errorCase(existPath(wPath)); // rFile == wPath の場合この辺で error();
	// rawKey

	midFile = makeTempFile(NULL);

	if (rawKey)
	{
		copyFile(rFile, midFile);

		LOGPOS();
		errorCase_m(!ZC_Decrypt(midFile, rawKey), "復号失敗");
		LOGPOS();
		ZC_Unpack(midFile, wPath);
		LOGPOS();
	}
	else
	{
		LOGPOS();
		ZC_Unpack(rFile, wPath);
		LOGPOS();
	}
	if (restoreDirMode)
	{
		removeFile(midFile);
		moveFile(wPath, midFile);

		LOGPOS();
		ZC_Uncluster(midFile, wPath);
		LOGPOS();
	}
	removeFile(midFile);
	memFree(midFile);
	PostOutput(rFile);
}

static void EncryptMain(autoBlock_t *rawKey, int mode) // mode: "ED"
{
	char *rFile;
	char *wFile;

	errorCase_m(!rawKey, "キーバンドルがセットされていません。@ EM");
	// mode

	rFile = nextArg();
	wFile = nextArg();

	errorCase(m_isEmpty(rFile));
	errorCase(m_isEmpty(wFile));
	errorCase(!existFile(rFile));
	errorCase(existPath(wFile)); // rFile == wFile の場合この辺で error();

	copyFile(rFile, wFile);

	if (mode == 'E')
	{
		ZC_Encrypt(wFile, rawKey);
	}
	else // D
	{
		if (!ZC_Decrypt(wFile, rawKey))
		{
			cout("+---------------------+\n");
			cout("| 復号に失敗しました。|\n");
			cout("+---------------------+\n");
			removeFile(wFile);
			termination(1);
		}
	}
	PostOutput(rFile);
	termination(0);
}

static void BackupFile(char *file)
{
	char *oldFile = addChar(strx(file), '_');

	cout("Backup File\n");
	cout("< %s\n", file);
	cout("> %s\n", oldFile);

	coExecute_x(xcout("DEL \"%s\"", oldFile));
	coExecute_x(xcout("REN \"%s\" \"%s\"", file, getLocal(oldFile)));

	memFree(oldFile);
}
int main(int argc, char **argv)
{
	autoBlock_t *rawKey = NULL;

readArgs:
	if (argIs("/KB"))
	{
		rawKey = cphrLoadKeyBundleFileEx(nextArg());
		goto readArgs;
	}
	if (argIs("/C"))
	{
		RestoreCurrentDirMode = 1;
		goto readArgs;
	}
	if (argIs("/1"))
	{
		MakeClusterFreeDirMode = 1;
		goto readArgs;
	}
	if (argIs("/-OW"))
	{
		NoOverwriteMode = 1;
		goto readArgs;
	}
	if (argIs("/OAD"))
	{
		cout("***********************\n");
		cout("** OUTPUT AND DELETE **\n");
		cout("***********************\n");

		OutputAndDeleteMode = 1;
		goto readArgs;
	}
	if (argIs("/M"))
	{
		char *rPath;
		char *wFile;

		rPath = nextArg();
		wFile = nextArg();

		MakeCluster(rPath, wFile, rawKey);
		return;
	}
	if (argIs("/MO"))
	{
		char *rPath;
		char *wFile;

		rPath = nextArg();
		wFile = nextArg();

		recurRemovePathIfExist(wFile);
		MakeCluster(rPath, wFile, rawKey);
		return;
	}
	if (argIs("/BM"))
	{
		char *rPath;
		char *wFile;

		rPath = nextArg();
		wFile = nextArg();

		BackupFile(wFile);
		MakeCluster(rPath, wFile, rawKey);
		return;
	}
	if (argIs("/RF"))
	{
		char *rFile;
		char *wPath;

		rFile = nextArg();
		wPath = nextArg();

		Restore(rFile, wPath, rawKey, 0);
		return;
	}
	if (argIs("/R"))
	{
		char *rFile;
		char *wDir;

		rFile = nextArg();
		wDir = nextArg();

		Restore(rFile, wDir, rawKey, 1);
		return;
	}
	if (argIs("/P"))
	{
		char *rFile;
		char *wFile;

		rFile = nextArg();
		wFile = nextArg();

		ZC_Pack(rFile, wFile);
		PostOutput(rFile);
		return;
	}
	if (argIs("/U"))
	{
		char *rFile;
		char *wFile;

		rFile = nextArg();
		wFile = nextArg();

		ZC_Unpack(rFile, wFile);
		PostOutput(rFile);
		return;
	}
	if (argIs("/E"))
	{
		EncryptMain(rawKey, 'E');
		return;
	}
	if (argIs("/D"))
	{
		EncryptMain(rawKey, 'D');
		return;
	}
	if (argIs("/E+"))
	{
		char *file = nextArg();

		ZC_Encrypt(file, rawKey);
//		PostOutput(file); // 入出力ファイルなので..
		return;
	}
	if (argIs("/D+"))
	{
		char *file = nextArg();

		if (!ZC_Decrypt(file, rawKey))
		{
			cout("+---------------------+\n");
			cout("| 復号に失敗しました。|\n");
			cout("+---------------------+\n");
			termination(1);
		}
//		PostOutput(file); // 入出力ファイルなので..
		termination(0);
	}
	if (argIs("/-I"))
	{
		ZC_WithoutInfo = 1;
		goto readArgs;
	}

	if (hasArgs(1))
	{
		AutoAction(nextArg(), rawKey);
		return;
	}
	for (; ; )
	{
		char *path = dropDirFile();

		AutoAction(path, rawKey);
		memFree(path);
		cout("\n");
	}
}
