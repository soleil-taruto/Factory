#include "C:\Factory\Common\all.h"

static void Test01(char *dir)
{
	updateDiskSpace_Dir(dir);

	cout("%I64u\n", lastDiskFree_User);
	cout("%I64u\n", lastDiskFree);
	cout("%I64u\n", lastDiskSize);
}
int main(int argc, char **argv)
{
	Test01("C:\\Nothing_Nothing_Nothing");
}
