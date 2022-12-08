/*
	.clu.gz.enc, ZC_Encrypt() とは異なる暗号化方式！

	----

	Encryptor.exe [/W KEY-WIDTH] [/P PASS-PHRASE | /B KEY-BUNDLE | /F KEY-FILE] [TARGET-FILE]...

		KEY-WIDTH   == 鍵の幅をバイト数で指定する。指定できるのは 16, 24, 32 のどれかで、デフォルトは 32
		PASS-PHRASE == パスフレーズを指定する。
		KEY-BUNDLE  == 鍵を16進数表記で指定する。
		KEY-FILE    == 鍵ファイル (* でパス入力, *PASS でパス指定, *PASS[x30] でパス拡張)
		TARGET-FILE == 暗号化・複号対象ファイル。

		鍵の長さは KEY-WIDTH の倍数であること。
		複号時、暗号化されたファイルが破損しているか鍵が間違っているとき、いつものエラーになる。
		TARGET-FILE を指定した場合、全ての TARGET-FILE を処理するとプログラムを終了する。(バッチモード)

		鍵ファイルの生成には mkcr.exe, p1k.exe, p2k.exe を使ってね。

		p1k.exe を使いたいが、ディスクに鍵を書き出すのは嫌だ。という場合は Encryptor.exe /f * を使ってね。
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
