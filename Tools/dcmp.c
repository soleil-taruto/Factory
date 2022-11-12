#include "C:\Factory\Common\all.h"

static void CompareDirPair(char *dir1, char *dir2, int ignoreCaseOfPath)
{
	errorCase(!existDir(dir1));
	errorCase(!existDir(dir2));

	if (isSameDir(dir1, dir2, ignoreCaseOfPath))
	{
		cout("+--------------+\n");
		cout("| ˆê’v‚µ‚Ü‚µ‚½ |\n");
		cout("+--------------+\n");
	}
	else
	{
		cout("+--------------------+\n");
		cout("| ˆê’v‚µ‚Ü‚¹‚ñ‚Å‚µ‚½ |\n");
		cout("+--------------------+\n");
		cout("* ‹Í‚©‚Èˆá‚¢‚Å‚ ‚ê‚Î dmd5 /c ƒRƒ}ƒ“ƒh‚Å“Á’è‚Å‚«‚é‚©‚à‚µ‚ê‚Ü‚¹‚ñ\n");
	}
}
int main(int argc, char **argv)
{
	char *dir1;
	char *dir2;
	int ignoreCaseOfPath= 0;

	if (argIs("/I")) // Ignore case of path
	{
		ignoreCaseOfPath = 1;
	}

	if (hasArgs(2))
	{
		dir1 = nextArg();
		dir2 = nextArg();

		CompareDirPair(dir1, dir2, ignoreCaseOfPath);
	}
	else
	{
		for (; ; )
		{
			dir1 = dropDir();
			dir2 = dropDir();

			CompareDirPair(dir1, dir2, ignoreCaseOfPath);
			cout("\n");
		}
	}
}
