/*
	.clu.gz.enc, ZC_Encrypt() �Ƃ͈قȂ�Í��������I

	----

	Encryptor.exe [/W KEY-WIDTH] [/P PASS-PHRASE | /B KEY-BUNDLE | /F KEY-FILE] [TARGET-FILE]...

		KEY-WIDTH   == ���̕����o�C�g���Ŏw�肷��B�w��ł���̂� 16, 24, 32 �̂ǂꂩ�ŁA�f�t�H���g�� 32
		PASS-PHRASE == �p�X�t���[�Y���w�肷��B
		KEY-BUNDLE  == ����16�i���\�L�Ŏw�肷��B
		KEY-FILE    == ���t�@�C�� (* �Ńp�X����, *PASS �Ńp�X�w��, *PASS[x30] �Ńp�X�g��)
		TARGET-FILE == �Í����E�����Ώۃt�@�C���B

		���̒����� KEY-WIDTH �̔{���ł��邱�ƁB
		�������A�Í������ꂽ�t�@�C�����j�����Ă��邩�����Ԉ���Ă���Ƃ��A�����̃G���[�ɂȂ�B
		TARGET-FILE ���w�肵���ꍇ�A�S�Ă� TARGET-FILE ����������ƃv���O�������I������B(�o�b�`���[�h)

		���t�@�C���̐����ɂ� mkcr.exe, p1k.exe, p2k.exe ���g���ĂˁB

		p1k.exe ���g���������A�f�B�X�N�Ɍ��������o���̂͌����B�Ƃ����ꍇ�� Encryptor.exe /f * ���g���ĂˁB
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Cipher.h"
#include "C:\Factory\Common\Options\Progress.h"
#include "C:\Factory\OpenSource\sha512.h"

#define EXT_CIPHER "enc01"
#define EXT_CIPHER_SUB "enc"

static void Interlude(void)
{
	Progress();

	while (hasKey())
	{
		if (getKey() == 0x1b)
		{
			ProgressEnd(1);
			cout("Cancelled.\n");
			termination(0);
		}
	}
}
int main(int argc, char **argv)
{
	uint kw = 32;
	char *kbfile;
	autoBlock_t *kb;
	autoList_t *ktlst;

	if (argIs("/W")) // key Width
	{
		kw = toValue(nextArg());
	}
	if (argIs("/P")) // Passphrase
	{
		sha512_makeHashLine(nextArg());
		kbfile = NULL;
		kb = recreateBlock(sha512_hash, 64);
		goto initktlst;
	}
	if (argIs("/B")) // key Bundle
	{
		kbfile = NULL;
		kb = makeBlockHexLine(nextArg());
		goto initktlst;
	}
	if (argIs("/F")) // key-bundle File
	{
		kbfile = strx(nextArg());
		goto initkb;
	}

	for (; ; )
	{
		int batchmode;

		cout("Input Key-Bundle file.\n");
		kbfile = dropFile();
		cout("\n");

	initkb:
		kb = cphrLoadKeyBundleFileEx(kbfile);
	initktlst:
		ktlst = cphrCreateKeyTableList(kb, kw);

		cout("Key Bundle Size: %u\n", getSize(kb));
		cout("Key Width: %u\n", kw);
		cout("Key Table Length: %u\n", getCount(ktlst));
		cout("\n");

		batchmode = hasArgs(1);

		for (; ; )
		{
			char *infile;
			char *outfile;
			int doEncrypt;
			int retval;

			if (batchmode)
			{
				if (!hasArgs(1))
					break;

				infile = strx(nextArg());
				cout("Input: %s\n", infile);
			}
			else
			{
				cout("Input Plain or Cipher(." EXT_CIPHER ") file.\n");
				infile = dropPath();
				cout("\n");

				if (!infile)
					break;
			}
			errorCase(!existFile(infile));

			if (
				!_stricmp(getExt(infile), EXT_CIPHER) ||
				!_stricmp(getExt(infile), EXT_CIPHER_SUB)
				)
			{
				outfile = changeExt(infile, "");
				doEncrypt = 0;
			}
			else
			{
				outfile = addExt(strx(infile), EXT_CIPHER);
				doEncrypt = 1;
			}
			cout("Output: %s\n", outfile);
			cout("Mode: %s\n", doEncrypt ? "Encrypt" : "Decrypt");
			cout("\n");

			cout("Processing...\n");
			ProgressBegin();

			if (doEncrypt)
			{
				cphrEncryptorFile(infile, outfile, ktlst, Interlude);
				retval = 1;
			}
			else
			{
				retval = cphrDecryptorFile(infile, outfile, ktlst, Interlude);
			}
			ProgressEnd(0);
			cout("Done!\n");
			cout("\n");

			if (!retval) // ? meybe fault decryption
			{
				if (existFile(outfile))
					removeFile(outfile);

				error();
			}
			memFree(infile);
			memFree(outfile);
		}
		memFree(kbfile);
		releaseAutoBlock(kb);
		cphrReleaseKeyTableList(ktlst);

		if (batchmode)
			break;
	}
	cout("\\e\n");
}
