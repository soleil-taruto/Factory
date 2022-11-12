#include "C:\Factory\Common\all.h"

static void CompareDirPair(char *dir1, char *dir2, int ignoreCaseOfPath)
{
	errorCase(!existDir(dir1));
	errorCase(!existDir(dir2));

	if (isSameDir(dir1, dir2, ignoreCaseOfPath))
	{
		cout("+--------------+\n");
		cout("| ��v���܂��� |\n");
		cout("+--------------+\n");
	}
	else
	{
		cout("+--------------------+\n");
		cout("| ��v���܂���ł��� |\n");
		cout("+--------------------+\n");
		cout("* �͂��ȈႢ�ł���� dmd5 /c �R�}���h�œ���ł��邩������܂���\n");
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
