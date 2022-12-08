/*
	�f�B���N�g�����t�@�C���ɂ܂Ƃ߂܂��B

	�f�t�H���g�œ��t�E�����͕ۑ����܂���B
	�����͕ۑ����܂���B

	----

	Cluster.exe [/C] [/Q] [/T] [/OAC] [/OAD] [/OW] [/E-] [/I] [/PP- ptPtn] [�N���X�^�t�@�C�� | ���̓f�B���N�g�� |
	             /K  �N���X�^�t�@�C�� |
	             /M  �o�̓N���X�^�t�@�C�� ����DIR |
	             /MO �o�̓N���X�^�t�@�C�� ����DIR |
	             /BM �o�̓N���X�^�t�@�C�� ����DIR |
	             /R  ���̓N���X�^�t�@�C�� �o��DIR |
	             /RQ ���̓N���X�^�t�@�C�� �o��DIR |
	             /MR ����DIR �o��DIR [���ԃt�@�C��]]

		/C   ... ��������̂Ƃ���ɓ��̓t�@�C���Ɠ����f�B���N�g���ɏo�͂���B
		/1   ... ��������̂Ƃ���Ƀt���[�f�B���N�g���ɏo�͂���B
		/Q   ... ���X�g�A���ɔj�����`�F�b�N���Ȃ��B
		/T   ... �g���X�g���[�h
		/OAC ... �o�͌�A���̓t�@�C�����폜����B���̓f�B���N�g���͋�ɂ���B
		/OAD ... �o�͌�A���̓t�@�C�����폜����B���̓f�B���N�g�����폜����B
		/OW  ... �o�͐悪���݂���ꍇ�㏑������B
		/K   ... �`�F�b�N�̂�
		/E-  ... ��������� C:\nnn �ɃN���X�^�t�@�C����W�J�����Ƃ��A�z���Ɉ���t�@�C�������������ꍇ�̓G�N�X�v���[�����J�����A�o�͐���폜����B
		/E-+ ... /E- �̌� FSqDiv /T DIR 10 ������B
		/E-2 ... /E- �̌� OrderStamp ������B
		/E-3 ... /E- �̌� PickOutDx ������B
		/I   ... �N���X�^�t�@�C���𐶐�����Ƃ��t�@�C�������ƃ^�C���X�^���v���ۑ�����B
		/PP- ... �p�X�� ptPtn ���܂ރf�B���N�g���E�t�@�C���𖳎�����B

		/M
			�N���X�^�t�@�C������
			�o�̓N���X�^�t�@�C�� ... ���݂��Ȃ��쐬�\�ȃp�X�ł��邱�ƁB
			����DIR

		/MO
			�N���X�^�t�@�C������ (�㏑��)
			�o�̓N���X�^�t�@�C�� ... �폜�\�ȃt�@�C�� or ���݂��Ȃ��쐬�\�ȃp�X�ł��邱�ƁB
			����DIR

		/BM
			�N���X�^�t�@�C������ (�㏑���{�P����ێ�)
			�o�̓N���X�^�t�@�C�� ... �폜�\�ȃt�@�C�� or ���݂��Ȃ��쐬�\�ȃp�X�ł��邱�ƁB
			����DIR

		/R
			�f�B���N�g������
			���̓N���X�^�t�@�C��
			�o��DIR ... ���݂��Ȃ��쐬�\�ȃp�X�ł��邱�ƁB

		/RQ
			�f�B���N�g������ (�`�F�b�N����)
			���̓N���X�^�t�@�C��
			�o��DIR ... ���݂��Ȃ��쐬�\�ȃp�X�ł��邱�ƁB

		/MR
			�f�B���N�g���̃R�s�[
			����DIR
			�o��DIR ... ���݂��Ȃ��쐬�\�ȃp�X�ł��邱�ƁB
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\SumStream.h"

static int IsNoFilesDir(char *dir)
{
	autoList_t *paths;
	int ret;

	findLimiter = 10; // tekitou
	paths = lssFiles(dir);
	findLimiter = 0;
	ret = getCount(paths) == 0;
	releaseDim(paths, 1);
	return ret;
}

static char *IgnorePtPtn;

static int IsNoIgnorePtPtn(char *path)
{
	char *path_a = xcout("%s*", path);
	int ret;

	ret = !mbs_stristr(path_a, IgnorePtPtn);

	memFree(path_a);
	return ret;
}

#define EXT_CLUSTER "clu"

static FILE *Stream;
static uint64 RWCount;
static int OutputAndCleanMode;
static int OutputAndDeleteMode;
static int OverwriteMode;

static void Writer(uchar *block, uint size)
{
	autoBlock_t writeBlock = gndBlock(block, size);

	writeBinaryBlock(Stream, &writeBlock);
	RWCount += size;

//	if (2 <= size) // old
	if (2 <= size && pulseSec(1, NULL))
		cout("\r%I64u bytes wrote OK", RWCount);
}
static void Reader(uchar *block, uint size)
{
	autoBlock_t *readBlock = readBinaryBlock(Stream, size);
	uint readSize;
	uint erred = 0;

	readSize = getSize(readBlock);

	if (readSize != size)
	{
		erred = 1;
		memset(block, 0x00, size);
	}
	else
	{
		memcpy(block, directGetBuffer(readBlock), readSize);
	}
	releaseAutoBlock(readBlock);
	RWCount += size;

//	if (2 <= size || erred) // old
	if (2 <= size && pulseSec(1, NULL) || erred)
		cout("\r%I64u bytes read %s", RWCount, erred ? "ERROR\n" : "OK");
}

static void BackupFile(char *file)
{
	char *oldFile = addChar(strx(file), '_');

	cout("Backup File\n");
	cout("< %s\n", file);
	cout("> %s\n", oldFile);

#if 1 // �ʂ̃v���Z�X���g�p���ł��B�΍� @ 2016.5.11
	// �����ꍇ�������B@ 2017.7.2
	if (existFile(file))
	{
		for (; ; )
		{
			coExecute_x(xcout("DEL \"%s\"", oldFile));

			if (!existFile(oldFile))
				break;

			coSleep(2000);
		}
		for (; ; )
		{
			coExecute_x(xcout("REN \"%s\" \"%s\"", file, getLocal(oldFile)));

			if (!existFile(file))
				break;

			coSleep(2000);
		}
	}
#else
	coExecute_x(xcout("DEL \"%s\"", oldFile));
	coExecute_x(xcout("REN \"%s\" \"%s\"", file, getLocal(oldFile)));
#endif

	memFree(oldFile);
}
static void MakeCluster(char *file, char *dir)
{
	cout("Make Cluster\n");
	cout("> %s\n", file);
	cout("< %s\n", dir);

	if (OverwriteMode)
	{
		cout("OW_DELETE DEST\n");
		recurRemovePathIfExist(file);
		cout("OW_DELETE DEST Ok.\n");
	}
	errorCase(existPath(file));
	errorCase(!existDir(dir));

	Stream = fileOpen(file, "wb");
	RWCount = 0;
	DirToSumStream(dir, Writer);
	fileClose(Stream);

	cout("\r%I64u bytes wrote FINISHED\n", RWCount);

	if (OutputAndCleanMode)
	{
		cout("CLEAN DIR\n");
		recurClearDir(dir); // HACK: ���t�@�C�����ɑΉ�����H
		cout("CLEAN DIR Ok.\n");
	}
	else if (OutputAndDeleteMode)
	{
		cout("DELETE DIR\n");
		recurRemoveDir(dir); // HACK: ���t�@�C�����ɑΉ�����H
		cout("DELETE DIR Ok.\n");
	}
}
static void RestoreCluster(char *file, char *dir)
{
	int retval;

	cout("Restore dir from Cluster\n");
	cout("< %s\n", file);
	cout("> %s\n", dir);

	if (OverwriteMode)
	{
		cout("OW_DELETE DEST\n");
		recurRemovePathIfExist(dir);
		cout("OW_DELETE DEST Ok.\n");
	}
	errorCase(!existFile(file));
	errorCase(existPath(dir));

	createDir(dir);

	Stream = fileOpen(file, "rb");
	RWCount = 0;
	retval = SumStreamToDir(dir, Reader);
	fileClose(Stream);

	cout("\r%I64u bytes read %s\n", RWCount, retval ? "SUCCESSFUL" : "FAULT");
	errorCase(!retval);

	if (OutputAndCleanMode || OutputAndDeleteMode)
	{
		cout("DELETE CLU\n");
		removeFile(file);
		cout("DELETE CLU Ok.\n");
	}
}
static void CheckCluster(char *file)
{
	int retval;
	uint64 fsize;

	cout("Check summary of Cluster\n");
	cout("< %s\n", file);

	errorCase(!existFile(file));

	fsize = getFileSize(file);

	Stream = fileOpen(file, "rb");
	RWCount = 0;
	retval = CheckSumStream(Reader, fsize);
	fileClose(Stream);

	cout("\r%I64u bytes read %s\n", RWCount, retval ? "CHECK SUM OK" : "CHECK SUM ERROR");
	errorCase(!retval);
}

static int RestoreSameDirMode;
static int RestoreFreeDirMode;
static int NoCheckClusterMode;
static int UnopenEmptyClusterMode;
static int FSqDivMode;
static int OrderStampMode;
static int PickOutDxMode;

static void AutoActCluster(char *path)
{
	if (existDir(path))
	{
		char *fdir = NULL;
		char *file;

		if (RestoreFreeDirMode)
		{
			fdir = makeFreeDir();
			file = combine_cx(fdir, addExt(strx(getLocal(path)), EXT_CLUSTER));
		}
		else
		{
			file = addExt(strx(path), EXT_CLUSTER);
		}

		MakeCluster(file, path);

		if (fdir)
		{
			execute_x(xcout("START %s\n", fdir));
			memFree(fdir);
		}
		memFree(file);
	}
	else
	{
		char *fdir = NULL;
		char *dir;

		errorCase(_stricmp(EXT_CLUSTER, getExt(path)));

		if (!RestoreSameDirMode)
		{
			fdir = makeFreeDir();
			dir = combine_cx(fdir, changeExt(getLocal(path), ""));
		}
		else
		{
			dir = changeExt(path, "");
		}

		if (!NoCheckClusterMode)
			CheckCluster(path);

		RestoreCluster(path, dir);

		if (fdir)
		{
			if (UnopenEmptyClusterMode && IsNoFilesDir(fdir))
			{
				LOGPOS();
				recurRemoveDir(fdir);
			}
			else if (FSqDivMode)
			{
				// memo: DSqDiv.exe �� dir �Ƃ͕ʂ̃t�H���_�ɏo�͂���B

				coExecute_x(xcout("C:\\Factory\\Tools\\FSqDiv.exe /T \"%s\" 5", dir));
			}
			else if (PickOutDxMode)
			{
				// memo: PickOutDx.exe �� dir �Ƃ͕ʂ̃t�H���_�ɏo�͂���B

				coExecute_x(xcout("C:\\Factory\\Labo\\Tools\\PickOutDx.exe \"%s\"", dir));
			}
			else
			{
				if (OrderStampMode)
					coExecute_x(xcout("C:\\Factory\\SubTools\\OrderStamp.exe \"%s\"", dir));

				execute_x(xcout("START %s\n", fdir));
			}
			memFree(fdir);
		}
		memFree(dir);
	}
}

int main(int argc, char **argv)
{
readArgs:
	if (argIs("/C")) // out to same dir (Current dir?) mode
	{
		RestoreSameDirMode = 1;
		goto readArgs;
	}
	if (argIs("/1")) // out to free dir mode
	{
		RestoreFreeDirMode = 1;
		goto readArgs;
	}
	if (argIs("/Q")) // Quick mode
	{
		NoCheckClusterMode = 1;
		goto readArgs;
	}
	if (argIs("/T")) // Trust mode
	{
		cout("***********\n");
		cout("** TRUST **\n");
		cout("***********\n");

		STD_TrustMode = 1;
		goto readArgs;
	}
	if (argIs("/OAC"))
	{
		cout("**********************\n");
		cout("** OUTPUT AND CLEAN **\n");
		cout("**********************\n");

		OutputAndCleanMode = 1;
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
	if (argIs("/OW"))
	{
		cout("***************\n");
		cout("** OVERWRITE **\n");
		cout("***************\n");

		OverwriteMode = 1;
		goto readArgs;
	}
	if (argIs("/E-"))
	{
		UnopenEmptyClusterMode = 1;
		goto readArgs;
	}
	if (argIs("/E-+"))
	{
		UnopenEmptyClusterMode = 1;
		FSqDivMode = 1;
		goto readArgs;
	}
	if (argIs("/E-2"))
	{
		UnopenEmptyClusterMode = 1;
		OrderStampMode = 1;
		goto readArgs;
	}
	if (argIs("/E-3"))
	{
		UnopenEmptyClusterMode = 1;
		PickOutDxMode = 1;
		goto readArgs;
	}
	if (argIs("/I"))
	{
		cout("+-------------------------------+\n");
		cout("| �����Ɠ������N���X�^�����܂��B|\n");
		cout("+-------------------------------+\n");

		DTS_WithInfo = 1;
		goto readArgs;
	}
	if (argIs("/PP-"))
	{
		char *ptPtn = nextArg();

		cout("[DTS]IgnorePtPtn: %s\n", ptPtn);

		IgnorePtPtn = ptPtn;
		DTS_AcceptPath = IsNoIgnorePtPtn;
		goto readArgs;
	}

	if (argIs("/M")) // Make cluster
	{
		MakeCluster(getArg(0), getArg(1)); // (0) �o�̓t�@�C�� <- (1) ����DIR, (0) ... ���݂��Ȃ��쐬�\�ȃp�X
		return;
	}
	if (argIs("/MO")) // Make cluster Overwrite
	{
		removeFileIfExist(getArg(0));
		MakeCluster(getArg(0), getArg(1)); // (0) �o�̓t�@�C�� <- (1) ����DIR, (0) ... �폜�\�ȃt�@�C�� or ���݂��Ȃ��쐬�\�ȃp�X
		return;
	}
	if (argIs("/BM")) // Backup and Make cluster
	{
		BackupFile(getArg(0));
		MakeCluster(getArg(0), getArg(1)); // (0) �o�̓t�@�C�� <- (1) ����DIR, (0) ... ���݂��Ȃ��쐬�\�ȃp�X
		return;
	}
	if (argIs("/K")) // Check cluster
	{
		CheckCluster(getArg(0));
		return;
	}
	if (argIs("/RQ")) // Restore dir from cluster
	{
		RestoreCluster(getArg(0), getArg(1)); // (0) ���̓t�@�C�� -> (1) �o��DIR, (1) ... ���݂��Ȃ��쐬�\�ȃp�X
		return;
	}
	if (argIs("/R")) // Check and Restore dir from cluster
	{
		CheckCluster(getArg(0));
		RestoreCluster(getArg(0), getArg(1)); // (0) ���̓t�@�C�� -> (1) �o��DIR, (1) ... ���݂��Ȃ��쐬�\�ȃp�X
		return;
	}
	if (argIs("/MR")) // Make and Restore (Copy)
	{
		char *inpDir = getArg(0); // (0) ����DIR -> (2) ���ԃt�@�C��_�ȗ��\ -> (1) �o��DIR, (1) ... ���݂��Ȃ��쐬�\�ȃp�X, (2) ... �㏑���\�ȃt�@�C�� or ���݂��Ȃ��쐬�\�ȃp�X
		char *outDir = getArg(1);
		char *midFile = hasArgs(3) ? strx(getArg(2)) : makeTempPath(NULL);

		cout("< %s\n", inpDir);
		cout("> %s\n", outDir);
		cout("+ %s\n", midFile);

		MakeCluster(midFile, inpDir);
		RestoreCluster(midFile, outDir);
		removeFile(midFile);
		memFree(midFile);
		return;
	}

	/*
	�N���X�^���h���b�v
		/C ���� -> �o�͐�͑��݂��Ȃ��쐬�\�ȃp�X
		/C �Ȃ� -> C:\1, 2, 3... �ɏo��
	�f�B���N�g�����h���b�v
		�o�͐�͑��݂��Ȃ��쐬�\�ȃp�X
	*/

	if (hasArgs(1))
	{
		AutoActCluster(getArg(0));
		return;
	}

	for (; ; )
	{
		char *path = dropPath();

		if (!path)
			break;

		AutoActCluster(path);
		memFree(path);
		cout("\n");
	}
}
