#include "C:\Factory\Common\all.h"
#include "C:\Factory\SubTools\libs\Mutector.h"

static int Interlude(void)
{
	return waitKey(0) != 0x1b;
}
static void Recved(autoBlock_t *message)
{
	char *line = ab_toLine(message);

	line2JLine(line, 1, 1, 1, 1);
	cout("[%s]\n", line);
	memFree(line);
}
int main(int argc, char **argv)
{
	Mutector_t *i = CreateMutector("Mutector_Test");

	LOGPOS();
	MutectorRecv(i, Interlude, Recved);
	LOGPOS();
	ReleaseMutector(i);
}
