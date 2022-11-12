#include "C:\Factory\Common\all.h"

static void Test_m_range(void)
{
	uint c;

	for (c = 1; c <= 100; c++)
	{
		uint d = c;
		m_range(d, 100, 200);
		errorCase(d != 100);
	}
	for (c = 100; c <= 200; c++)
	{
		uint d = c;
		m_range(d, 100, 200);
		errorCase(d != c);
	}
	for (c = 200; c <= 300; c++)
	{
		uint d = c;
		m_range(d, 100, 200);
		errorCase(d != 200);
	}
	cout("OK!\n");
}

int main(int argc, char **argv)
{
	Test_m_range();
}
