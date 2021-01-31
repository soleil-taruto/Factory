#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	char *path;

	cout("+----------+\n");
	cout("| Snapshot |\n");
	cout("+----------+\n");

	path = dropDirFile();

	coExecute_x(xcout("C:\\app\\Rico\\Snapshot.exe \"%s\"", path));

	memFree(path);
}
