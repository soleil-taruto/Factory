#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	char *rFile;

	cout("+--------------------------------------+\n");
	cout("| 最初にドロップされたファイルの内容を |\n");
	cout("| ２番目以降にドロップされたファイルに |\n");
	cout("| 上書きします。                       |\n");
	cout("+--------------------------------------+\n");

	rFile = dropFile();

	for (; ; )
	{
		char *wFile = dropFile();

		cout("< %s\n", rFile);
		cout("> %s\n", wFile);

		copyFile(rFile, wFile);

		cout("OK\n");

		memFree(wFile);
	}
	memFree(rFile);
}
