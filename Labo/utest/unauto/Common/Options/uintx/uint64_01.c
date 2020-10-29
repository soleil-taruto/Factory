#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\uintx\uint64.h"

static void Test01(void)
{
	UI64_t a = UI64_x(IMAX);
	UI64_t b = UI64_x(IMAX);
	UI64_t c;

	c = UI64_Mul(a, b, NULL);

	cout("%I64u\n", (uint64)c.H.Value << 32 | c.L.Value);
}
static void Test02(void)
{
	UI64_t ret = UI64_Div(
		UI64_Mul(UI64_x(IMAX), UI64_x(IMAX), NULL),
		UI64_x(IMAX),
		NULL
		);

	cout("%I64u\n", (uint64)ret.H.Value << 32 | ret.L.Value);
}
int main(int argc, char **argv)
{
	Test01();
	Test02();

	cout("OK!\n");
}
