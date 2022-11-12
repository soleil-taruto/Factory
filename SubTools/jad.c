/*
	jad.exe /D CLASS-FILE

		... CLASS-FILE ���f�R���p�C������ changeExt( CLASS-FILE, "java" ) �𐶐�����B
			�C���i�[�N���X�Ƃ� CLASS-FILE �̃p�X������T���B
			CLASS-FILE �̃t�@�C�����E�g���q�͏o�L�ڂł����Ă��ǂ��B

	jad.exe /DJ CLASS-FILE JAVA-FILE

		... CLASS-FILE ���f�R���p�C������ JAVA-FILE �𐶐�����B
			�C���i�[�N���X�Ƃ� CLASS-FILE �̃p�X������T���B
			CLASS-FILE �̃t�@�C�����E�g���q�͏o�L�ڂł����Ă��ǂ��B
*/

#include "C:\Factory\Common\all.h"

/*
	Jad 1.5.8g for Windows 9x/NT/2000 on Intel platform
	�󔒂��܂܂Ȃ��p�X�ł��邱�ƁB
*/
#define JAD_LOCAL_FILE "jad.exe"
#define JAD_FILE "C:\\app\\jad158g.win\\" JAD_LOCAL_FILE

static char *GetJadFile(void)
{
	static char *file;

	if (!file)
	{
		file = JAD_LOCAL_FILE;

		if (!existFile(file))
		{
			file = JAD_FILE;
			errorCase_m(!existFile(file), "[" JAD_FILE "]��������܂���B");
		}
	}
	return file;
}
static void ClassToJava(char *classFile, char *javaFile)
{
	char *dir = makeTempDir(NULL);

	classFile = makeFullPath(classFile);
	javaFile = makeFullPath(javaFile);

	cout("< %s\n", classFile);
	cout("> %s\n", javaFile);

	errorCase(!existFile(classFile));
	createPath(javaFile, 'F'); // �o�͐�DIR�쐬 + �������݃e�X�g
	removeFile(javaFile);

	addCwd(dir);
	coExecute_x(xcout("%s \"%s\"", GetJadFile(), classFile));

	{
		autoList_t *files = lsFiles(".");

		if (getCount(files) == 1)
		{
			moveFile(getLine(files, 0), javaFile);
		}
		else
		{
			FILE *fp = fileOpen(javaFile, "wt");
			char *file;
			uint index;

			writeLine(fp, "���s���܂����B");
			writeLine_x(fp, xcout("�N���X�t�@�C��=%s", classFile));

			foreach (files, file, index)
				writeLine_x(fp, xcout("�o�̓t�@�C��=%s", file));

			fileClose(fp);
		}
		releaseDim(files, 1);
	}

	unaddCwd();

	removeDir(dir);
	memFree(dir);

	memFree(classFile);
	memFree(javaFile);
}
int main(int argc, char **argv)
{
	if (argIs("/D"))
	{
		char *classFile;
		char *javaFile;

		classFile = nextArg();
		javaFile = changeExt(classFile, "java");

		ClassToJava(classFile, javaFile);

		memFree(javaFile);
		return;
	}
	if (argIs("/DJ"))
	{
		char *classFile;
		char *javaFile;

		classFile = nextArg();
		javaFile = nextArg();

		ClassToJava(classFile, javaFile);
		return;
	}
}
