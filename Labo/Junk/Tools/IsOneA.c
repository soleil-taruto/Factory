#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	int ret = argc == 2 && strlen(argv[1]) == 1 && m_islower(argv[1][0]) ? 0 : 1;

	ret = m_01(ret); // –â‘èƒAƒŠ -> 1

	cout("ret: %d\n", ret);

	return ret;
}
