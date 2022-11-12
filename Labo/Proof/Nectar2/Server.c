#include "C:\Factory\Common\all.h"
#include "C:\Factory\SubTools\libs\Nectar2.h"

int main(int argc, char **argv)
{
	Nectar2_t *i = CreateNectar2("Kira_Kira_PRE-CURE_a_la_mode");

	while (waitKey(0) != 0x1b)
	{
		char *message = Nectar2RecvLine(i, '\0');

		if (message)
		{
			cout("%s\n", message);
			memFree(message);
		}
	}
	ReleaseNectar2(i);
}
