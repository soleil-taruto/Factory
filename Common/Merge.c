#include "all.h"

/*
	list1, list2 から共通する要素を取り出して、そのリスト mList を返す。
	list1, list2 はソートする。
	list1, list2, mList は本関数終了時に全てソートされている。

	同じ要素について、list1 の要素は mList に追加、list2 の要素は開放する。
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
	merge は strcmp で行われる。
	秀丸だとタブ・空白の変換や行末の空白の削除でも「削除行」扱いになってしまうことに注意せよ。
	同一行があっても問題無い。

	最初にエディタに渡すリストは並べ替えしない。
	絞込みしたリストはソートする。
	戻り値はソートする。
*/
autoList_t *selectLines(autoList_t *lines) // linesを変更せず、新しいリストを返す。NULLを返すことはない。
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

	cout("選択項目をテキストエディタで開きました。\n");
	cout("選択したい行あるいは選択したい行以外を変更して下さい。\n");
	execute(selfile); // ブロッキングでファイル編集
	cout("テキストエディタが終了しました。\n");

	// 秀丸は、空にして保存すると削除できる。
	if (!existFile(selfile))
		createFile(selfile);

	lLines = copyLines(lines);
	eLines = readLines(selfile);
	rLines = mergeLines(lLines, eLines);

	releaseDim(eLines, 1);

	cout("-------------------------------------------------------------------------------\n");
	cout("R = 変更されなかった行 %u\n", getCount(rLines));
	cout("L = 削除・変更された行 %u\n", getCount(lLines));
	cout("r = 変更されなかった行だけを残して、再編集\n");
	cout("l = 削除・変更された行だけを残して、再編集\n");
	cout("Space = 再編集\n");
	cout("Escape = キャンセル\n");
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
		cout("再編集\n");
		releaseDim(lLines, 1);
		releaseDim(rLines, 1);
		goto reedit;

	case 0x1b: // Escape
		cout("キャンセル\n");
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
char *selectLine(autoList_t *lines) // ret == NULL: キャンセルした。
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
		cout("一つの行を選択して下さい！\n");
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

autoList_t *autoDistinctLines(autoList_t *lines) // lines: 開放する。
{
	rapidSortLines(lines); // strcmp2 == strcmp (-> simpleComp 廃止)

	{
		autoList_t *result = newList();
		autoList_t *dust = newList();

		distinctLines(lines, result, dust); // strcmp

		releaseAutoList(lines);
		releaseDim(dust, 1);

		return result;
	}
}
autoList_t *autoDistinctJLinesICase(autoList_t *lines) // lines: 開放する。
{
	rapidSortJLinesICase(lines); // strcmp3 == mbs_stricmp -> strcmp (-> simpleComp 廃止)

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
