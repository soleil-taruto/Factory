#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	time_t t = time(NULL);
	time_t b;
	uint n = now();

	b = t - n;

	cout("���ݓ��� %s\n", makeJStamp(getStampDataTime(t), 0));
	cout("�N������ %s\n", makeJStamp(getStampDataTime(b), 0));

	{
		uint d = n;
		uint h;
		uint m;
		uint s;

		s = d % 60; d /= 60;
		m = d % 60; d /= 60;
		h = d % 24; d /= 24;

		cout("�N�����Ă��� %u ���� %02u ���� %02u �� %02u �b (%u �b) �o�߂��܂����B\n", d, h, m, s, n);
	}
}
