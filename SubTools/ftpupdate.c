/*
	Windows7では動かない。mdel * あたりで凍る。
*/
#include "C:\Factory\Common\all.h"

#define SCRIPT "C:\\Factory\\tmp\\ftpupdate_script.txt"

static char *ServerName;
static char *UserName;
static char *Passphrase;

static void UploadCurrDir(char *dir)
{
	FILE *fp = fileOpen(SCRIPT, "wt");

	writeLine_x(fp, xcout("open %s", ServerName));
	writeLine(fp, UserName);
	writeLine(fp, Passphrase);
	writeLine(fp, "prompt");

	if (*dir != '.')
	{
		autoList_t *ptkns = tokenize(dir, '\\'); // 漢字は使わないだろう。
		char *ptkn;
		uint index;

		foreach (ptkns, ptkn, index)
		{
			writeLine_x(fp, xcout("mkdir %s", ptkn));
			writeLine_x(fp, xcout("cd %s", ptkn));
		}
	}
	writeLine(fp, "mdel");
	writeLine(fp, "*");
	writeLine(fp, "bin");
	writeLine(fp, "mput");
	writeLine(fp, "*");
	writeLine(fp, "quit");

	fileClose(fp);

	addCwd(dir);
	execute_x(xcout("ftp -s:%s", SCRIPT));
	unaddCwd();
}
static void FtpUpdate(void)
{
	autoList_t *dirs = lssDirs(".");
	char *dir;
	uint index;

	dir = getCwd();
	eraseRoots(dirs, dir);
	memFree(dir);

	UploadCurrDir(".");

	foreach (dirs, dir, index)
	{
		UploadCurrDir(dir);
	}
	releaseDim(dirs, 1);
}
int main(int argc, char **argv)
{
	ServerName = nextArg();
	UserName = nextArg();
	Passphrase = nextArg();

	if (hasArgs(1))
	{
		addCwd(nextArg());
		FtpUpdate();
		unaddCwd();
	}
	else
	{
		FtpUpdate();
	}
}
