#include "C:\Factory\Common\all.h"
#include "C:\Factory\OpenSource\sha512.h"

static void DoTest(char *text, char *answer)
{
	sha512_makeHashLine(text);
	sha512_makeHexHash();

	cout("TEXT=%s\n", text);
	cout("HASH=%s\n", sha512_hexHash);
	cout("ANSH=%s\n", answer);

	errorCase(_stricmp(sha512_hexHash, answer));
}
int main(int argc, char **argv)
{
	// test-vector from: http://www.febooti.com/products/filetweak/members/hash-and-crc/test-vectors/

	DoTest(
		"",
		"cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e"
		);
	DoTest(
		"Test vector from febooti.com",
		"09fb898bc97319a243a63f6971747f8e102481fb8d5346c55cb44855adc2e0e98f304e552b0db1d4eeba8a5c8779f6a3010f0e1a2beb5b9547a13b6edca11e8a"
		);
	DoTest(
		"The quick brown fox jumps over the lazy dog",
		"07e547d9586f6a73f73fbac0435ed76951218fb7d0c8d788a309d785436bbb642e93a252a954f23912547d1e8a3b5ed6e1bfd7097821233fa0538f3db854fee6"
		);

	cout("OK!\n");
}
