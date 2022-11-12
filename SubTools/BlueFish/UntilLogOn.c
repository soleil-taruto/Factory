/*
	�I�͂P���
	���O�I����ʂ͉摜�A�f�X�N�g�b�v�͒P�F�Ȃ̂ŁA
	�X�N���[���L���v�`�������摜�� (0, 0) �` (99, 0) ���S�ē����F�ł���΃f�X�N�g�b�v�ƌ��Ȃ��B
	�f�X�N�g�b�v�̍���ɂ��ݔ�������͂��B
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Meteor\Toolkit.h"
#include "C:\Factory\SubTools\libs\bmp.h"

static int IsSingleColor_First100Px_BmpRow(autoList_t *bmpRow)
{
	uint x;

	for (x = 1; x < 100; x++)
		if (refElement(bmpRow, 0) != refElement(bmpRow, x))
			return 0;

	return 1;
}
static int IsLoggedOn(void)
{
	char *dir = makeTempDir(NULL);
	int ret = 0;

	addCwd(dir);
	{
		coExecute(FILE_TOOLKIT_EXE " /PRINT-SCREEN ss_");

		if (existFile("ss_01.bmp")) // �t�@�C�������� == �X�N���[���V���b�g���B��Ȃ��ꍇ�́A�p�X���[�h���͒� (Secure Desktop) �ƍl������B
		{
			autoList_t *bmp = readBMPFile("ss_01.bmp");
			autoList_t *bmpRow;
			uint x;

			LOGPOS();
			bmpRow = refList(bmp, 0);

			if (IsSingleColor_First100Px_BmpRow(bmpRow))
			{
				LOGPOS();
				bmpRow = refList(bmp, 35); // ����ɂ͂��ݔ�������͂��B������� (���̒i���P�F�Ȃ�) �u�����N��ʂ����H

				if (!IsSingleColor_First100Px_BmpRow(bmpRow))
				{
					LOGPOS();
					ret = 1;
				}
			}
			releaseDim_BR(bmp, 2, NULL);
		}
	}
	unaddCwd();

	recurRemoveDir_x(dir);
	return ret;
}
int main(int argc, char **argv)
{
	uint exitCode = 0;

	errorCase(!existFile(FILE_TOOLKIT_EXE)); // �O���R�}���h���݊m�F

	while (!IsLoggedOn())
	{
		int key = coWaitKey(5000);

		if (key == 0x0d)
			break;

		if (key == 0x1b)
		{
			exitCode = 1;
			break;
		}
		clearKey();
	}
	cout("exitCode: %u\n", exitCode);
	termination(exitCode);
}
