#include "C:\Factory\Common\all.h"
#include "C:\Factory\OpenSource\mt19937\mt19937ar.h"

static void Test_mt19937(void)
{
	char *outfile = makeTempFile("mt19937-test-out");
	autoList_t *out1;
	autoList_t *out2;

	coExecute_x(xcout("> \"%s\" \"%s\" /ORIGINAL-MAIN", outfile, getSelfFile()));

	out1 = readLines(outfile);
	out2 = readLines("testvector\\mt19937ar.out");

	errorCase(!isSameLines(out1, out2, 0));

	releaseDim(out1, 1);
	releaseDim(out2, 1);
	removeFile(outfile);
	memFree(outfile);

	cout("OK\n");
}

int main(int argc, char **argv)
{
	if (argIs("/ORIGINAL-MAIN"))
	{
		mt19937_main();
		return;
	}
	Test_mt19937();
}
