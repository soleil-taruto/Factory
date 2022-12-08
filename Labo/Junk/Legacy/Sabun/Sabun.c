#include "C:\Factory\Common\all.h"
#include "libs\Sabun.h"

int main(int argc, char **argv)
{
	int correctAddDelete = 0;

	if (argIs("/CAD")) // Correct Add Delete
	{
		correctAddDelete = 1;
	}

	if (argIs("/M")) // Make sabun
	{
		char *beforeDir;
		char *afterDir;
		char *outDir;
		char *sabunFile;

		cout("+----------------------------+\n");
		cout("| �ύX�O�̃t�H���_���h���b�v |\n");
		cout("+----------------------------+\n");
		beforeDir = dropDir();

		cout("+----------------------------+\n");
		cout("| �ύX��̃t�H���_���h���b�v |\n");
		cout("+----------------------------+\n");
		afterDir = dropDir();

		outDir = makeTempDir(NULL);
		sabunFile = combine(outDir, "Sabun.bin");

		makeSabun(sabunFile, beforeDir, afterDir, correctAddDelete);

		execute_x(xcout("START %s", outDir));

		memFree(beforeDir);
		memFree(afterDir);
		memFree(outDir);
		memFree(sabunFile);
	}
	else // �����K�p
	{
		char *targetDir;
		char *sabunFile;

		cout("+----------------------------------+\n");
		cout("| �Ώۃt�H���_���h���b�v           |\n");
		cout("| * ���̃t�H���_�̒��g���X�V���܂� |\n");
		cout("+----------------------------------+\n");
		targetDir = dropDir();

		cout("+------------------------+\n");
		cout("| �����t�@�C�����h���b�v |\n");
		cout("+------------------------+\n");
		sabunFile = dropFile();

		cout("\n");
		cout("�Ώۃt�H���_��ύX���܂��B\n");
		cout("�������J�n���Ă���̓L�����Z���ł��܂���B\n");
		cout("���s����ɂ̓G���^�[�L�[�������ĂˁB\n");

		if (clearGetKey() == 0x0d)
		{
			cout("\n");
			cout("�A�b�v�f�[�g���Ă��܂�...");

			if (sabunUpdate(sabunFile, targetDir) == 0) // ? �A�b�v�f�[�g�ΏۊO�������B
			{
				cout("\r");
				cout("+----------------------------+\n");
				cout("| �G���[�^�A�b�v�f�[�g�ΏۊO |\n");
				cout("+----------------------------+\n");
			}
			else
			{
				cout("\r�A�b�v�f�[�g�͊������܂����B\n");
			}
			clearWaitKey(5000); // ������悤��
		}
		memFree(targetDir);
		memFree(sabunFile);
	}
}
