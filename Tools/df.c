#include "C:\Factory\Common\all.h"

static void DispDiskFree(int drive, char *dir)
{
	updateDiskSpace(drive);

	cout("%c�h���C�u\n", m_toupper(drive));
	cout("�f�B�X�N�g�p�� = %.3f %%\n", (lastDiskSize - lastDiskFree) * 100.0 / lastDiskSize);
	cout("�f�B�X�N�g�p = %s �o�C�g\n", c_thousandComma(xcout("%I64u", lastDiskSize - lastDiskFree)));
	cout("�f�B�X�N�e�� = %s �o�C�g\n", c_thousandComma(xcout("%I64u", lastDiskSize)));
	cout("�f�B�X�N�� = %s �o�C�g\n", c_thousandComma(xcout("%I64u", lastDiskFree)));
	cout("���p�s�̈� = %s �o�C�g\n", c_thousandComma(xcout("%I64u", lastDiskFree - lastDiskFree_User)));
	cout("���p�\�̈� = %s �o�C�g\n", c_thousandComma(xcout("%I64u", lastDiskFree_User)));

	if (dir)
	{
		updateDiskSpace_Dir(dir);

		cout("���p�\�̈� = %s �o�C�g (�f�B���N�g��)\n", c_thousandComma(xcout("%I64u", lastDiskFree_User)));
	}
}
int main(int argc, char **argv)
{
	if (argIs("*"))
	{
		char drive[] = "_:\\";

		for (drive[0] = 'A'; drive[0] <= 'Z'; drive[0]++)
			if (existDir(drive))
				DispDiskFree(drive[0], NULL);

		return;
	}

	if (hasArgs(1))
	{
		char *arg = nextArg();

		if (strlen(arg) == 1)
		{
			DispDiskFree(arg[0], NULL);
		}
		else
		{
			char *dir = makeFullPath(arg);

			DispDiskFree(dir[0], dir);

			memFree(dir);
		}
	}
	else
	{
		char *dir = c_getCwd();

		DispDiskFree(dir[0], dir);
	}
}
