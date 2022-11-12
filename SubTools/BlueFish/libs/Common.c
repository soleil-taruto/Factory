#include "Common.h"

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
