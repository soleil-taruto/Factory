// smpl
/*
	DateTime2Sec.exe /DT2S ���\�[�X����

		���\�[�X���� == MMDD, YYYY, YYYYMM, YYYYMMDD, YYYYMMDDhh, YYYYMMDDhhmm, YYYYMMDDhhmmss
		�Ή����� 0001/01/01 00:00:00 ����̌o�ߕb����\������B

	DateTime2Sec.exe /S2DT �b��

		�b�� == 0001/01/01 00:00:00 ����̌o�ߕb��
		�Ή����� YYYYMMDDhhmmss ��\������B

	Datetime2Sec.exe PRM

		PRM < 10000101000000 �̏ꍇ

			PRM �� 0001/01/01 00:00:00 ����̌o�ߕb���ƌ��Ȃ��āA
			�Ή����� YYYYMMDDhhmmss ��\������B

		PRM >= 10000101000000 �̏ꍇ

			PRM �� YYYYMMDDhhmmss �ƌ��Ȃ��āA
			�Ή����� 0001/01/01 00:00:00 ����̌o�ߕb����\������B
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\TimeData.h"

int main(int argc, char **argv)
{
	if (argIs("/DT2S"))
	{
		cout("%I64u\n", TimeData2TSec(Res2TimeData(nextArg())));
		return;
	}
	if (argIs("/S2DT"))
	{
		cout("%I64u\n", TimeData2Stamp(TSec2TimeData(toValue64(nextArg()))));
		return;
	}

	{
		uint64 prm = toValue64(nextArg());

		if (prm < 10000101000000ui64)
		{
			cout("%I64u\n", TimeData2Stamp(TSec2TimeData(prm)));
		}
		else
		{
			cout("%I64u\n", TimeData2TSec(Stamp2TimeData(prm)));
		}
	}
}
