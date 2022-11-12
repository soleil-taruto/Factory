/*
	time_t ���Z�b�g����Ƃ��́AApplyStampData(getStampDataTime(t)); �Ƃ���B
	LastStampData ���X�V����邱�Ƃɒ��ӂ���B
*/
#include "ApplyStampData.h"

void ApplyStampData(stampData_t *i)
{
	/*
		23:59:59.999�Ƃ��V�r�A�ȃ^�C�~���O�������肷��ƁA
		DATE���s -> ���t���ς�� -> TIME���s�ŁA�ۈ�����ꂽ�肷�邩���B
		�Ȃ̂�TIME���ɁB
	*/
	coExecute_x(xcout("TIME %u.%u.%u", i->hour, i->minute, i->second));
	coExecute_x(xcout("DATE %u.%u.%u", i->year, i->month, i->day));
}
void SlewApplyTimeData(time_t trueTime)
{
	time_t systemTime = time(NULL);
	time_t t;

	cout("%I64d -> %I64d\n", systemTime, trueTime); // systemTime ���� trueTime �ɋ߂Â���B

	if (trueTime == systemTime) // �����������Ă���B-> �������Ȃ��B
	{
		cout("+0.0\n");
		goto endFunc;
	}

	do
	{
//		sleep(100);
		sleep(10);
	}
	while (systemTime == time(NULL));

	sleep(500); // systemTime + 1.5 �܂ő҂B

	if (trueTime < systemTime) // �V�X�e���������i��ł���B-> 0.5 �x�点��B(systemTime + 1.5 -> systemTime + 1.0) == -0.5
	{
		cout("-0.5\n");
		ApplyStampData(getStampDataTime(systemTime + 1));
	}
	else if (systemTime < trueTime) // �V�X�e���������x��Ă���B-> 0.5 �i�߂�B(systemTime + 1.5 -> systemTime + 2.0) == +0.5
	{
		cout("+0.5\n");
		ApplyStampData(getStampDataTime(systemTime + 2));
	}
endFunc:;
}
