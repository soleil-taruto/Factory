#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	char szRoot[16];
	char szVolume[256];
	char szSystem[256];
	uint dwSerial;
	uint dwLength;
	uint dwFlags;

	// ドライブの設定
	strcpy(szRoot, "C:\\");

	if (hasArgs(1))
	{
		int drv = nextArg()[0];

		errorCase(!m_isalpha(drv));
		szRoot[0] = drv;
	}

	// ボリューム情報の取得
	errorCase(!GetVolumeInformation(
		szRoot,
		szVolume, sizeof(szVolume),
		&dwSerial,
		&dwLength,
		&dwFlags,
		szSystem, sizeof(szSystem)
		));
		// 成功すると 0 以外を、それ以外は 0 を返す。

	cout("Drive: %s\n", szRoot);
	cout("Volume: %s\n", szVolume);
	cout("Serial: %08x\n", dwSerial);
	cout("F/S: %s\n", szSystem);
	cout("Flag: %08x\n", dwFlags);
	cout("MaxLength: %u\n", dwLength);
}
