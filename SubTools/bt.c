/*
	bt == batch tool
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\CRRandom.h"

int main(int argc, char **argv)
{
	uint retval = 0;

	mt19937_initCRnd();

	if (argIs("NOW"))
	{
		cout("%s\n", c_makeJStamp(NULL, 0));
		goto endFunc;
	}
endFunc:
	termination(retval);
}
