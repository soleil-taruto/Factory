#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	time_t t = time(NULL);
	time_t b;
	uint n = now();

	b = t - n;

	cout("現在日時 %s\n", makeJStamp(getStampDataTime(t), 0));
	cout("起動日時 %s\n", makeJStamp(getStampDataTime(b), 0));

	{
		uint d = n;
		uint h;
		uint m;
		uint s;

		s = d % 60; d /= 60;
		m = d % 60; d /= 60;
		h = d % 24; d /= 24;

		cout("起動してから %u 日と %02u 時間 %02u 分 %02u 秒 (%u 秒) 経過しました。\n", d, h, m, s, n);
	}
}
