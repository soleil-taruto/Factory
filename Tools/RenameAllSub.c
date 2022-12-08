/*
	RenameAllSub.exe [�Ώۃt�H���_]
*/

#include "C:\Factory\Common\all.h"

static autoList_t *PubFiles1;
static autoList_t *PubFiles2;

static void MovePubFiles(void)
{
	char *path;
	uint index;

	foreach (PubFiles1, path, index)
	{
		cout("- %s\n", path);
		cout("+ %s\n", getLine(PubFiles2, index));
		moveFile(path, getLine(PubFiles2, index));
	}
}
static int RenumberPaths(autoList_t *paths)
{
	char *path;
	uint index;
	uint rencnt = 0;

	foreach (paths, path, index)
	{
		char *snum = xcout("%010u", index + 1);
		char *p;
		char *q;

		reverseLine(path);
		reverseLine(snum);
		q = snum;

		for (p = path; *p; p++)
		{
			if (*p == '*') // �����ɒu��������B
			{
				if (*q)
				{
					*p = *q;
					q++;
				}
				else
				{
					*p = '0';
				}
				rencnt++;
			}
		}
		reverseLine(path);
		memFree(snum);
	}
	return rencnt;
}
static int AntiSamePaths(autoList_t *paths)
{
	char *path;
	uint index;
	autoList_t gal;
	uint inccnt = 0;

	foreach (paths, path, index)
	{
		while (findJLineICase(gndSubElementsVar(paths, 0, index, gal), path) < index) // ? found same path
		{
			path = incrementPath(path);
			setElement(paths, index, (uint)path);
			inccnt++;
		}
	}
	return inccnt;
}
static int Confirm(void)
{
	cout("Press R to continue.\n");

	for (; ; )
	{
		int key = getKey();

		if (key == 0x1b) return 0;
		if (key == 'R') return 1;
	}
}

static void RenameEx(void)
{
	autoList_t *paths = lss(".");
	autoList_t *newPaths;
	autoList_t *tmpPaths;
	char *path;
	uint index;

	rapidSort(paths, (sint (*)(uint, uint))mbs_stricmp);
	newPaths = copyLines(paths);

	for (; ; )
	{
		tmpPaths = editTextLines(newPaths);
		releaseDim(newPaths, 1);
		newPaths = tmpPaths;

		if (getCount(newPaths) == 0)
		{
			goto endfunc;
		}
		if (getCount(newPaths) < getCount(paths))
		{
			for (index = 0; getCount(newPaths) < getCount(paths); index++)
				addElement(newPaths, (uint)strx(getLine(newPaths, index)));
		}
		else if (getCount(paths) < getCount(newPaths))
		{
			while (getCount(paths) < getCount(newPaths))
				memFree((void *)unaddElement(newPaths));
		}
		else if (RenumberPaths(newPaths))
		{
			// noop
		}
		else if (AntiSamePaths(newPaths))
		{
			// noop
		}
		else
		{
			break;
		}
	}
	foreach (paths, path, index) // �ύX���Ȃ��t�@�C����r��
	{
		if (!strcmp(path, getLine(newPaths, index))) // ? ���S�Ɉ�v
		{
			path[0] = '\0';
			getLine(newPaths, index)[0] = '\0';
		}
	}
	trimLines(paths);
	trimLines(newPaths);

	foreach (paths, path, index)
	{
		cout("< %s\n", path);
		cout("> %s\n", getLine(newPaths, index));

		errorCase(!existFile(path)); // ? ! �t�@�C�� // HACK: �t�H���_�͖��Ή�

		// Check
		{
			char *tmp = toFairFullPathFltr(getLine(newPaths, index));
			cout("$ %s\n", tmp);
			errorCase(strcmp(tmp, getLine(newPaths, index))); // ? ! ���S�Ɉ�v
			memFree(tmp);
		}
	}
	if (!Confirm())
		goto endfunc;

	tmpPaths = createAutoList(getCount(paths));

	foreach (paths, path, index)
	{
		addElement(tmpPaths, (uint)makeTempPath(NULL));
	}
	foreach (paths, path, index) // �����ړ�
	{
		cout("< %s\n", path);
		cout("? %s\n", getLine(tmpPaths, index));
		moveFile(path, getLine(tmpPaths, index));
		moveFile(getLine(tmpPaths, index), path); // �߂�
	}
	foreach (paths, path, index)
	{
		cout("< %s\n", path);
		cout("* %s\n", getLine(tmpPaths, index));
		moveFile(path, getLine(tmpPaths, index));
	}

	// �����ړ�
	{
		PubFiles1 = tmpPaths;
		PubFiles2 = paths;
		addFinalizer(MovePubFiles);

		foreach (tmpPaths, path, index)
		{
			cout("* %s\n", path);
			cout("? %s\n", getLine(newPaths, index));
			moveFile(path, getLine(newPaths, index));
			moveFile(getLine(newPaths, index), path); // �߂�
		}
		unaddFinalizer(MovePubFiles);
		PubFiles1 = NULL;
		PubFiles2 = NULL;
	}

	foreach (tmpPaths, path, index)
	{
		cout("* %s\n", path);
		cout("> %s\n", getLine(newPaths, index));
		moveFile(path, getLine(newPaths, index));
	}
	releaseDim(tmpPaths, 1);

endfunc:
	releaseDim(paths, 1);
	releaseDim(newPaths, 1);
}
static void RenameExDir(char *dir)
{
	addCwd(dir);
	RenameEx();
	unaddCwd();
}

int main(int argc, char **argv)
{
	errorCase_m(getArg(0)[0] == '/', "�I�v�V�����w��\�ł���Ɗ��Ⴂ���Ă��܂��񂩁H");

	if (hasArgs(1))
	{
		RenameExDir(nextArg());
	}
	else
	{
		char *dir = dropDir();

		RenameExDir(dir);
		memFree(dir);
	}
}
