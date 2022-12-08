#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\rbTree.h"
#include "LCom.h"

rbTree_t *Dictionary;

static char *RecvEvent(char *line)
{
	autoList_t *tokens = tokenizeMinMax(line, '=', 1, 2, NULL);
	char *resLine;

	cout("Recv: %s\n", line);

	if (getCount(tokens) == 2)
	{
		char *key   = getLine(tokens, 0);
		char *value = getLine(tokens, 1);

		if (rbtHasKey(Dictionary, (uint)key))
		{
			memFree((void *)rbtUnaddValue(Dictionary, (uint)key));
		}
		rbtAddValue(Dictionary, (uint)key, (uint)strx(value));
		resLine = "Settled";
	}
	else if (getCount(tokens) == 1)
	{
		char *key = getLine(tokens, 0);
		char *value;

		if (rbtHasKey(Dictionary, (uint)key))
		{
//			value = (char *)rbtGetValue(Dictionary, (uint)key);
			value = (char *)rbtGetLastAccessValue(Dictionary);
		}
		else
		{
			value = "Undefined";
		}
		resLine = value;
	}
	else
	{
		error();
	}

	cout("Send: %s\n", resLine);

	resLine = strx(resLine);
	return resLine;
}
int main(int argc, char **argv)
{
	Dictionary = rbCreateTree((uint (*)(uint))strx, (sint (*)(uint, uint))strcmp, (void (*)(uint))memFree);

	LComRecvLoop(RecvEvent, 1);

	rbtCallAllValue(Dictionary, (void (*)(uint))memFree);
	rbReleaseTree(Dictionary);
}
