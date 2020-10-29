/*
	VwPw.exe [PASSWORD]
*/

#include "C:\Factory\Common\all.h"

static void ViewPassword(char *pw)
{
	uint pwLen = strlen(pw);
	uint index;

	if(!pwLen)
		return;

	errorCase(!isAsciiLine(pw, 0, 0, 0));

	cout("\n");

	for(index = 0; index < pwLen; index++)
	{
		if(index && index % 4 == 0)
			cout(" ");

		cout("%c", pw[index]);
	}
	cout("\n");

	for(index = 0; index < pwLen; index++)
	{
		uint c = index % 8 / 4;
		uint d = index / 8 % 2;

		if(index && index % 4 == 0)
			cout(" ");

		cout("%c", "=- "[c ? d : 2]);
	}
	cout("\n");

	for(index = 0; index < pwLen; index++)
	{
		uint c = index % 16 / 12;

		if(index && index % 4 == 0)
			cout(" ");

		cout("%c", " #"[c]);
	}
	cout("\n");
}
int main(int argc, char **argv)
{
	if(hasArgs(1))
	{
		ViewPassword(nextArg());
	}
	else
	{
		ViewPassword(inputLine());
	}
}
