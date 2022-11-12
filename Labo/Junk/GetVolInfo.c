#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	char szRoot[16];
	char szVolume[256];
	char szSystem[256];
	uint dwSerial;
	uint dwLength;
	uint dwFlags;

	// �h���C�u�̐ݒ�
	strcpy(szRoot, "C:\\");

	if (hasArgs(1))
	{
		int drv = nextArg()[0];

		errorCase(!m_isalpha(drv));
		szRoot[0] = drv;
	}

	// �{�����[�����̎擾
	errorCase(!GetVolumeInformation(
		szRoot,
		szVolume, sizeof(szVolume),
		&dwSerial,
		&dwLength,
		&dwFlags,
		szSystem, sizeof(szSystem)
		));
		// ��������� 0 �ȊO���A����ȊO�� 0 ��Ԃ��B

	cout("Drive: %s\n", szRoot);
	cout("Volume: %s\n", szVolume);
	cout("Serial: %08x\n", dwSerial);
	cout("F/S: %s\n", szSystem);
	cout("Flag: %08x\n", dwFlags);
	cout("MaxLength: %u\n", dwLength);
}
