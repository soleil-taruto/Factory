#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\csv.h"
#include "libs\BcpReader.h"

static int ArgToChar(char *arg)
{
	switch (strlen(arg))
	{
	case 1:
		return arg[0];

	case 2:
		errorCase(!lineExp("<2,09AFaf>", arg));
		return toValueDigits(arg, hexadecimal);

	default:
		error();
	}
	error(); // never
	return -1; // never
}
int main(int argc, char **argv)
{
	int chrT;
	int chrR;
	char *bcpFile;
	char *csvFile;
	autoList_t *table;

	chrT = ArgToChar(nextArg());
	chrR = ArgToChar(nextArg());
	bcpFile = nextArg();
	csvFile = nextArg();

	cout("T %02x\n", chrT);
	cout("R %02x\n", chrR);
	cout("< %s\n", bcpFile);
	cout("> %s\n", csvFile);

	table = SqlBcpReader(bcpFile, chrT, chrR);
	writeCSVFile_cx(csvFile, table);
}
