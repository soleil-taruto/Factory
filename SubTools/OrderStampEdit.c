/*
	タイムスタンプのソート順が指定された順になるようにタイムスタンプを変更する。
	★元のタイムスタンプの更新日時になるべく合わせる。

	OrderStampEdit.exe [対象ディレクトリ]
*/

#include "C:\Factory\Common\all.h"

typedef struct FileInfo_st
{
	char *File;
	time_t OldStamp;
	time_t Stamp;

	// -- EditOrder --

	uint EO_Index;

	// -- ChangeStamps --

	// none

	// --
}
FileInfo_t;

static sint CompStamp(uint v1, uint v2)
{
	FileInfo_t *a = (FileInfo_t *)v1;
	FileInfo_t *b = (FileInfo_t *)v2;
	sint ret;

	ret = simpleComp64((uint64)a->Stamp, (uint64)b->Stamp);

	if (ret)
		return ret;

	ret = strcmp(a->File, b->File);
	return ret;
}
static sint CompEOIndex(uint v1, uint v2)
{
	FileInfo_t *a = (FileInfo_t *)v1;
	FileInfo_t *b = (FileInfo_t *)v2;

	return simpleComp(a->EO_Index, b->EO_Index);
}
static void EditOrder(autoList_t *fileInfos)
{
	autoList_t *files = newList();
	autoList_t *filesTmp;
	char *file;
	FileInfo_t *i;
	FileInfo_t *i2;
	uint index;
	uint index2;

	foreach (fileInfos, i, index)
		addElement(files, (uint)i->File);

	filesTmp = editLines(files);
	releaseAutoList(files);
	files = filesTmp;
	trimLines(files); // 空行除去
	errorCase_m(getCount(files) != getCount(fileInfos), "編集エラー：ファイル数が増減した。");

	foreach (fileInfos, i, index)
	{
		i->EO_Index = findLine(files, i->File);
		errorCase_m(i->EO_Index == getCount(fileInfos), "編集エラー：ファイル消失");
	}
	foreach (fileInfos, i,  index)
	foreach (fileInfos, i2, index2)
	if (index < index2)
	{
		errorCase_m(i->EO_Index == i2->EO_Index, "編集エラー：ファイル重複");
	}
	rapidSort(fileInfos, CompEOIndex);

	releaseAutoList(files);
}
static uint CS_ChangeTest_L(autoList_t *fileInfos, uint index) // スタンプ変更「テスト」前方向, ret: 変更する件数
{
	time_t t = ((FileInfo_t *)getElement(fileInfos, index))->Stamp;
	uint ret = 0;

	while (index)
	{
		FileInfo_t *i = (FileInfo_t *)getElement(fileInfos, --index);

		t--;

		if (i->Stamp <= t) // ? 変更不要
			break;

		if (i->Stamp == i->OldStamp) // ? 変更されていない。-- 既に変更されていれば変更する件数としてカウントしない。
			ret++;
	}
	return ret;
}
static uint CS_ChangeTest_R(autoList_t *fileInfos, uint index) // スタンプ変更「テスト」後方向, ret: 変更する件数
{
	time_t t = ((FileInfo_t *)getElement(fileInfos, index))->Stamp;
	uint ret = 0;

	while (++index < getCount(fileInfos))
	{
		FileInfo_t *i = (FileInfo_t *)getElement(fileInfos, index);

		t++;

		if (t <= i->Stamp) // ? 変更不要
			break;

		ret++;
	}
	return ret;
}
static void CS_Change_L(autoList_t *fileInfos, uint index) // スタンプ変更_前方向
{
	time_t t = ((FileInfo_t *)getElement(fileInfos, index))->Stamp;

	while (index)
	{
		FileInfo_t *i = (FileInfo_t *)getElement(fileInfos, --index);

		t--;

		if (i->Stamp <= t) // ? 変更不要
			break;

		i->Stamp = t;
	}
}
static void CS_Change_R(autoList_t *fileInfos, uint index) // スタンプ変更_後方向
{
	time_t t = ((FileInfo_t *)getElement(fileInfos, index))->Stamp;

	while (++index < getCount(fileInfos))
	{
		FileInfo_t *i = (FileInfo_t *)getElement(fileInfos, index);

		t++;

		if (t <= i->Stamp) // ? 変更不要
			break;

		i->Stamp = t;
	}
}
static void ChangeStamps(autoList_t *fileInfos)
{
	FileInfo_t *i;
	uint index;

	foreach (fileInfos, i, index)
	if (index)
	{
		FileInfo_t *i1;
		FileInfo_t *i2;
		uint ndx1 = index - 1;
		uint ndx2 = index;

		i1 = (FileInfo_t *)getElement(fileInfos, ndx1);
		i2 = i;

		if (i2->Stamp <= i1->Stamp) // ? i1->Stamp < i2->Stamp ではない -> 要スタンプ変更
		{
			uint c1 = CS_ChangeTest_L(fileInfos, ndx2);
			uint c2 = CS_ChangeTest_R(fileInfos, ndx1);

			if (c1 < c2)
				CS_Change_L(fileInfos, ndx2);
			else
				CS_Change_R(fileInfos, ndx1);
		}
	}
}
static void DoOrderStampEdit(void)
{
	autoList_t *files = lsFiles(".");
	char *file;
	uint index;
	autoList_t *fileInfos = newList();

	LOGPOS();

	foreach (files, file, index)
	{
		FileInfo_t *i = nb_(FileInfo_t);
		uint64 stamp;
		time_t t;

		i->File = getLocal(file);

		getFileStamp(i->File, NULL, NULL, &stamp);
		t = getTimeByFileStamp(stamp);

		i->OldStamp = t;
		i->Stamp = t;

		addElement(fileInfos, (uint)i);
	}
	rapidSort(fileInfos, CompStamp);
	EditOrder(fileInfos);
	ChangeStamps(fileInfos);

	// test
	{
		FileInfo_t *i;

		foreach (fileInfos, i, index)
		if (index)
		{
			FileInfo_t *i1 = (FileInfo_t *)getElement(fileInfos, index - 1);
			FileInfo_t *i2 = i;

			errorCase(i2->Stamp <= i1->Stamp); // ? i1->Stamp < i2->Stamp ではない -> スタンプ更新失敗
		}
	}

	{
		FileInfo_t *i;
		int changing = 0;

		foreach (fileInfos, i, index)
		{
			if (i->OldStamp != i->Stamp)
			{
				uint64 oldStamp = getFileStampByTime(i->OldStamp);
				uint64 stamp    = getFileStampByTime(i->Stamp);

				cout("%I64u -> %I64u : %s\n", oldStamp, stamp, i->File);

				changing = 1;
			}
		}
		if (!changing)
			cout("タイムスタンプの更新はありません。\n");
	}

	// Confirm
	{
		cout("タイムスタンプを更新します。(全てのファイルに setFileStamp を実行します)\n");
		cout("続行？\n");

		if (clearGetKey() == 0x1b)
			termination(0);

		cout("続行します。\n");
	}

	{
		FileInfo_t *i;

		foreach (fileInfos, i, index)
		{
			uint64 stamp = getFileStampByTime(i->Stamp);

			cout("%I64u -> %s\n", stamp, i->File);

			setFileStamp(i->File, stamp, stamp, stamp);
		}
	}

	releaseDim(files, 1);
	releaseDim(fileInfos, 1);

	LOGPOS();
}
int main(int argc, char **argv)
{
	addCwd(hasArgs(1) ? nextArg() : c_dropDir());
	DoOrderStampEdit();
	unaddCwd();
}
