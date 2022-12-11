#include "Common.h"

#if 0 // ”p~ @ 2022.12.11

static int Real_IsBlueFishComputer(void)
{
	return existDir("C:\\BlueFish");
}
int IsBlueFishComputer(void)
{
	static int inited;
	static int ret;

	if (!inited)
	{
		ret = Real_IsBlueFishComputer();
		cout("‚±‚ÌŠÂ‹«‚Í BlueFish ‚Å%s\n", ret ? "‚·B" : "‚Í‚ ‚è‚Ü‚¹‚ñB");
		inited = 1;
	}
	return ret;
}

#else

int IsBlueFishComputer(void)
{
	return 0;
}

#endif
