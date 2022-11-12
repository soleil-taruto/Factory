#include "C:\Factory\Common\all.h"
#include "C:\Factory\SubTools\libs\Nectar.h"

static void Recved(autoBlock_t *message)
{
	char *line = ab_toLine(message);

	line2JLine(line, 1, 1, 1, 1);
	cout("[%s]\n", line);
	memFree(line);
}
int main(int argc, char **argv)
{
	Nectar_t *i = CreateNectar("Nectar_Test");

	LOGPOS();

	while (waitKey(0) != 0x1b)
	{
		autoBlock_t *message = NectarReceipt(i);

		if (message)
		{
			Recved(message);
			releaseAutoBlock(message);
		}
	}
	LOGPOS();
	ReleaseNectar(i);
}
