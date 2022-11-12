/*
	dropCompact.exe [/U]
*/

#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	char *path;

	if (argIs("/U"))
	{
		for (; ; )
		{
			cout("+----------+\n");
			cout("| à≥èkâèú |\n");
			cout("+----------+\n");

			path = dropDirFile();

			if (existDir(path))
			{
				coExecute_x(xcout("Compact.exe /U /S:\"%s\"", path));
			}
			else // file
			{
				coExecute_x(xcout("Compact.exe /U \"%s\"", path));
			}
			cout("\n");
		}
		error(); // never
	}

	{
		for (; ; )
		{
			cout("+------+\n");
			cout("| à≥èk |\n");
			cout("+------+\n");

			path = dropDirFile();

			if (existDir(path))
			{
				coExecute_x(xcout("Compact.exe /C /S:\"%s\"", path));
			}
			else // file
			{
				coExecute_x(xcout("Compact.exe /C \"%s\"", path));
			}
			cout("\n");
		}
		error(); // never
	}
}
