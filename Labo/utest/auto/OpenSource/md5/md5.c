#include "C:\Factory\Common\all.h"
#include "C:\Factory\OpenSource\md5.h"

static void DoTest(char *text, char *answer)
{
	char *hash = c_md5_makeHexHashLine(text);

	cout("TEXT=%s\n", text);
	cout("HASH=%s\n", hash);
	cout("ANSH=%s\n", answer);

	errorCase(_stricmp(hash, answer));
}
int main(int argc, char **argv)
{
	// test-vector from: http://www.febooti.com/products/filetweak/members/hash-and-crc/test-vectors/

	DoTest("", "d41d8cd98f00b204e9800998ecf8427e");
	DoTest("Test vector from febooti.com", "500ab6613c6db7fbd30c62f5ff573d0f");
	DoTest("The quick brown fox jumps over the lazy dog", "9e107d9d372bb6826bd81d3542a419d6");

	cout("OK!\n");
}
