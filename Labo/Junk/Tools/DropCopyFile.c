#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	char *rFile;

	cout("+--------------------------------------+\n");
	cout("| �ŏ��Ƀh���b�v���ꂽ�t�@�C���̓��e�� |\n");
	cout("| �Q�Ԗڈȍ~�Ƀh���b�v���ꂽ�t�@�C���� |\n");
	cout("| �㏑�����܂��B                       |\n");
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
