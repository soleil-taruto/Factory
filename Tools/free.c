#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	updateMemory();

	cout("�������g�p�� = %u %%\n", lastMemoryLoad);
	cout("������������ = %s �o�C�g\n", c_thousandComma(xcout("%I64u", lastMemoryFree)));
	cout("�����������e�� = %s �o�C�g\n", c_thousandComma(xcout("%I64u", lastMemorySize)));
	cout("���z�������� = %s �o�C�g\n", c_thousandComma(xcout("%I64u", lastVirtualFree)));
	cout("���z�������g�� = %s �o�C�g\n", c_thousandComma(xcout("%I64u", lastExVirtualFree)));
	cout("���z�������e�� = %s �o�C�g\n", c_thousandComma(xcout("%I64u", lastVirtualSize)));
	cout("�y�[�W�T�C�Y�� = %s �o�C�g\n", c_thousandComma(xcout("%I64u", lastPageFileFree)));
	cout("�y�[�W�T�C�Y�ő� = %s �o�C�g\n", c_thousandComma(xcout("%I64u", lastPageFileSize)));
}
