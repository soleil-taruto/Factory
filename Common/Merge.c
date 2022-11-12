#include "all.h"

/*
	list1, list2 ���狤�ʂ���v�f�����o���āA���̃��X�g mList ��Ԃ��B
	list1, list2 �̓\�[�g����B
	list1, list2, mList �͖{�֐��I�����ɑS�ă\�[�g����Ă���B

	�����v�f�ɂ��āAlist1 �̗v�f�� mList �ɒǉ��Alist2 �̗v�f�͊J������B
*/
autoList_t *merge(autoList_t *list1, autoList_t *list2, sint (*funcComp)(uint, uint), void (*funcRelease)(uint))
{
	autoList_t *aList1 = createAutoList(getCount(list1));
	autoList_t *aList2 = createAutoList(getCount(list2));
	autoList_t *mList = createAutoList(m_min(getCount(list1), getCount(list2)));

	rapidSort(list1, funcComp);
	rapidSort(list2, funcComp);

	while (getCount(list1) && getCount(list2))
	{
		int comp = funcComp(getLastElement(list1), getLastElement(list2));

		if (comp < 0) // <
		{
			addElement(aList2, unaddElement(list2));
		}
		else if (0 < comp) // >
		{
			addElement(aList1, unaddElement(list1));
		}
		else
		{
			addElement(mList, unaddElement(list1));
			funcRelease(unaddElement(list2));
		}
	}
	reverseElements(aList1);
	reverseElements(aList2);
	reverseElements(mList);

	addElements(list1, aList1);
	addElements(list2, aList2);

	releaseAutoList(aList1);
	releaseAutoList(aList2);

	return mList;
}
autoList_t *mergeLines(autoList_t *lines1, autoList_t *lines2)
{
	return merge(lines1, lines2, (sint (*)(uint, uint))strcmp, (void (*)(uint))memFree);
}
autoList_t *mergeConstLines(autoList_t *lines1, autoList_t *lines2)
{
	return merge(lines1, lines2, (sint (*)(uint, uint))strcmp, (void (*)(uint))noop);
}
autoList_t *mergeLinesICase(autoList_t *lines1, autoList_t *lines2)
{
	return merge(lines1, lines2, (sint (*)(uint, uint))mbs_stricmp, (void (*)(uint))memFree);
}
autoList_t *mergeConstLinesICase(autoList_t *lines1, autoList_t *lines2)
{
	return merge(lines1, lines2, (sint (*)(uint, uint))mbs_stricmp, (void (*)(uint))noop);
}

/*
	merge �� strcmp �ōs����B
	�G�ۂ��ƃ^�u�E�󔒂̕ϊ���s���̋󔒂̍폜�ł��u�폜�s�v�����ɂȂ��Ă��܂����Ƃɒ��ӂ���B
	����s�������Ă���薳���B

	�ŏ��ɃG�f�B�^�ɓn�����X�g�͕��בւ����Ȃ��B
	�i���݂������X�g�̓\�[�g����B
	�߂�l�̓\�[�g����B
*/
autoList_t *selectLines(autoList_t *lines) // lines��ύX�����A�V�������X�g��Ԃ��BNULL��Ԃ����Ƃ͂Ȃ��B
{
	char *selfile = makeTempPath("txt");
	autoList_t *eLines;
	autoList_t *lLines;
	autoList_t *rLines;
	autoList_t *retLines;

	cout("===============================================================================\n");

	lines = copyLines(lines);
reedit:
	writeLines(selfile, lines);

	cout("�I�����ڂ��e�L�X�g�G�f�B�^�ŊJ���܂����B\n");
	cout("�I���������s���邢�͑I���������s�ȊO��ύX���ĉ������B\n");
	execute(selfile); // �u���b�L���O�Ńt�@�C���ҏW
	cout("�e�L�X�g�G�f�B�^���I�����܂����B\n");

	// �G�ۂ́A��ɂ��ĕۑ�����ƍ폜�ł���B
	if (!existFile(selfile))
		createFile(selfile);

	lLines = copyLines(lines);
	eLines = readLines(selfile);
	rLines = mergeLines(lLines, eLines);

	releaseDim(eLines, 1);

	cout("-------------------------------------------------------------------------------\n");
	cout("R = �ύX����Ȃ������s %u\n", getCount(rLines));
	cout("L = �폜�E�ύX���ꂽ�s %u\n", getCount(lLines));
	cout("r = �ύX����Ȃ������s�������c���āA�ĕҏW\n");
	cout("l = �폜�E�ύX���ꂽ�s�������c���āA�ĕҏW\n");
	cout("Space = �ĕҏW\n");
	cout("Escape = �L�����Z��\n");
	cout("-------------------------------------------------------------------------------\n");

reinput:
	switch (clearGetKey())
	{
	case 'R':
		cout("R\n");
		releaseDim(lLines, 1);
		retLines = rLines;
		break;

	case 'L':
		cout("L\n");
		retLines = lLines;
		releaseDim(rLines, 1);
		break;

	case 'r':
		cout("r\n");
		releaseDim(lines, 1);
		releaseDim(lLines, 1);
		lines = rLines;
		goto reedit;

	case 'l':
		cout("l\n");
		releaseDim(lines, 1);
		lines = lLines;
		releaseDim(rLines, 1);
		goto reedit;

	case 0x20: // Space
		cout("�ĕҏW\n");
		releaseDim(lLines, 1);
		releaseDim(rLines, 1);
		goto reedit;

	case 0x1b: // Escape
		cout("�L�����Z��\n");
		retLines = createAutoList(0);
		releaseDim(lLines, 1);
		releaseDim(rLines, 1);
		break;

	default:
		goto reinput;
	}

	cout("===============================================================================\n");

	removeFile(selfile);
	memFree(selfile);
	releaseDim(lines, 1);
	return retLines;
}
char *selectLine(autoList_t *lines) // ret == NULL: �L�����Z�������B
{
	char *retLine;

	for (; ; )
	{
		autoList_t *selLines = selectLines(lines);

		if (getCount(selLines) == 0)
		{
			releaseDim(selLines, 1);
			retLine = NULL;
			break;
		}
		if (getCount(selLines) == 1)
		{
			retLine = getLine(selLines, 0);
			releaseAutoList(selLines);
			break;
		}
		cout("��̍s��I�����ĉ������I\n");
		releaseDim(selLines, 1);
	}
	return retLine;
}

autoList_t *merge2_bothExist2; // extra-prm

/*
	(list1, list2): sorted
*/
void merge2(autoList_t *list1, autoList_t *list2, sint (*funcComp)(uint, uint), autoList_t *onlyExist1, autoList_t *bothExist, autoList_t *onlyExist2)
{
	uint p = 0;
	uint q = 0;

	for (; ; )
	{
		uint e1 = p == getCount(list1);
		uint e2 = q == getCount(list2);
		sint comp;

		if (e1 && e2)
		{
			break;
		}

		if (e1)
		{
			comp = 1; // p > q
		}
		else if (e2)
		{
			comp = -1; // p < q
		}
		else
		{
			comp = funcComp(getElement(list1, p), getElement(list2, q));
		}

		if (comp < 0) // p < q
		{
			if (onlyExist1)
			{
				addElement(onlyExist1, getElement(list1, p));
			}
			p++;
		}
		else if (0 < comp) // p > q
		{
			if (onlyExist2)
			{
				addElement(onlyExist2, getElement(list2, q));
			}
			q++;
		}
		else // p == q
		{
			if (bothExist)
			{
				addElement(bothExist, getElement(list1, p));
			}
			if (merge2_bothExist2)
			{
				addElement(merge2_bothExist2, getElement(list2, q));
			}
			p++;
			q++;
		}
	}
}
/*
	(lines1, lines2): sorted by rapidSortLines()
*/
void mergeLines2(autoList_t *lines1, autoList_t *lines2, autoList_t *onlyExist1, autoList_t *bothExist, autoList_t *onlyExist2)
{
	merge2(lines1, lines2, (sint (*)(uint, uint))strcmp, onlyExist1, bothExist, onlyExist2);
}
/*
	(lines1, lines2): sorted by rapidSortJLinesICase(), sortJLinesICase()
*/
void mergeLines2ICase(autoList_t *lines1, autoList_t *lines2, autoList_t *onlyExist1, autoList_t *bothExist, autoList_t *onlyExist2)
{
	merge2(lines1, lines2, (sint (*)(uint, uint))mbs_stricmp, onlyExist1, bothExist, onlyExist2);
}

/*
	list: sorted
*/
void distinct(autoList_t *list, sint (*funcComp)(uint, uint), autoList_t *result, autoList_t *others)
{
	uint index;

	for (index = 0; index < getCount(list); index++)
	{
		if (index == 0 || funcComp(getElement(list, index), getElement(list, index - 1)))
		{
			if (result)
				addElement(result, getElement(list, index));
		}
		else
		{
			if (others)
				addElement(others, getElement(list, index));
		}
	}
}
void distinctLines(autoList_t *lines, autoList_t *result, autoList_t *others)
{
	distinct(lines, (sint (*)(uint, uint))strcmp, result, others);
}
void distinctJLinesICase(autoList_t *lines, autoList_t *result, autoList_t *others)
{
	distinct(lines, (sint (*)(uint, uint))mbs_stricmp, result, others);
}

autoList_t *autoDistinctLines(autoList_t *lines) // lines: �J������B
{
	rapidSortLines(lines); // strcmp2 == strcmp (-> simpleComp �p�~)

	{
		autoList_t *result = newList();
		autoList_t *dust = newList();

		distinctLines(lines, result, dust); // strcmp

		releaseAutoList(lines);
		releaseDim(dust, 1);

		return result;
	}
}
autoList_t *autoDistinctJLinesICase(autoList_t *lines) // lines: �J������B
{
	rapidSortJLinesICase(lines); // strcmp3 == mbs_stricmp -> strcmp (-> simpleComp �p�~)

	{
		autoList_t *result = newList();
		autoList_t *dust = newList();

		distinctJLinesICase(lines, result, dust); // mbs_stricmp

		releaseAutoList(lines);
		releaseDim(dust, 1);

		return result;
	}
}

void distinct2(autoList_t *list, sint (*funcComp)(uint, uint), void (*funcRelease)(uint))
{
	uint index;
	uint wi = getCount(list) ? 1 : 0;

	rapidSort(list, funcComp);

	for (index = 1; index < getCount(list); index++)
	{
		if (!funcComp(getElement(list, wi - 1), getElement(list, index)))
			funcRelease(getElement(list, index));
		else
			setElement(list, wi++, getElement(list, index));
	}
	setCount(list, wi);
}

// _x
autoList_t *selectLines_x(autoList_t *lines)
{
	autoList_t *r = selectLines(lines);
	releaseDim(lines, 1);
	return r;
}
char *selectLine_x(autoList_t *lines)
{
	char *r = selectLine(lines);
	releaseDim(lines, 1);
	return r;
}
