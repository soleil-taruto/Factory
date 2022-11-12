/*
	adjustPackage.exe [/B] [�p�b�P�[�W�̃��[�g�f�B���N�g�� �ϊ����s���\�[�X�̃f�B���N�g��]

		�ϊ����s���\�[�X�̃f�B���N�g�� �� �p�b�P�[�W�̃��[�g�f�B���N�g�� �̔z���ł��邱�ƁB�����ł����Ă͂Ȃ�Ȃ��B
*/

#include "C:\Factory\Common\all.h"

static int BatchMode;

static char *GetPackageFromLine(char *line)
{
	char *package = strx(line + 8);

	ne_strchr(package, ';')[0] = '\0';
	return package;
}
static char *GetPackageFromPath(char *packageRootDir, char *javaFile)
{
	char *javaDir = changeLocal(javaFile, "");
	char *relJavaDir;
	char *package;

	relJavaDir = changeRoot(javaDir, packageRootDir, NULL);
	package = strx(relJavaDir);
	replaceChar(package, '\\', '.');
	return package;
}
static void AdjustPackage(char *packageRootDir, char *srcDir)
{
	autoList_t *files;
	char *file;
	uint index;
	char *relSrcDir;

	packageRootDir = makeFullPath(packageRootDir);
	srcDir = makeFullPath(srcDir);

	cout("P %s\n", packageRootDir);
	cout("S %s\n", srcDir);

	relSrcDir = changeRoot(strx(srcDir), packageRootDir, NULL); // srcDir �� packageRootDir �̔z���ł��邱�ƁB

	cout("R %s\n", relSrcDir);

	if (!BatchMode)
	{
		cout("���s�H\n");

		if (clearGetKey() == 0x1b)
			termination(0);

		cout("���s���܂��B\n");
	}
	files = lssFiles(srcDir);

	foreach (files, file, index)
	{
		if (!_stricmp("java", getExt(file)))
		{
			autoList_t *lines = readLines(file);
			char *line;
			uint index;
			int modified = 0;

			foreach (lines, line, index)
			{
				if (startsWith(line, "package "))
				{
					char *oldPackage = GetPackageFromLine(line);
					char *newPackage = GetPackageFromPath(packageRootDir, file);

					cout("< %s\n", oldPackage);
					cout("> %s\n", newPackage);

					if (strcmp(oldPackage, newPackage))
					{
						LOGPOS();
						memFree(line);
						line = xcout("package %s;", newPackage);
						setElement(lines, index, (uint)line);
						modified = 1;
					}
					memFree(oldPackage);
					memFree(newPackage);
				}
			}
			if (modified)
			{
				LOGPOS();
				semiRemovePath(file);
				writeLines(file, lines);
			}
			releaseDim(lines, 1);
		}
	}
	releaseDim(files, 1);
	memFree(packageRootDir);
	memFree(srcDir);
	memFree(relSrcDir);
}
int main(int argc, char **argv)
{
	if (argIs("/B"))
	{
		BatchMode = 1;
	}

	if (hasArgs(2))
	{
		AdjustPackage(getArg(0), getArg(1));
		return;
	}

	{
		char *packageRootDir;
		char *srcDir;

		cout("[Java]�p�b�P�[�W�̃��[�g�f�B���N�g���F\n");
		packageRootDir = dropDir();

		cout("[Java]�ϊ����s���\�[�X�̃f�B���N�g���F\n");
		srcDir = dropDir();

		AdjustPackage(packageRootDir, srcDir);

		memFree(packageRootDir);
		memFree(srcDir);
	}
}
