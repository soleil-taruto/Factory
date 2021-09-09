/*
	�^�C���X�^���v�̃\�[�g�����w�肳�ꂽ���ɂȂ�悤�Ƀ^�C���X�^���v��ύX����B
	�����̃^�C���X�^���v�̍X�V�����ɂȂ�ׂ����킹��B

	OrderStampEdit.exe [�Ώۃf�B���N�g��]
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

	// TODO

	// --
}
FileInfo_t;

static sint CompStamp(uint v1, uint v2)
{
	FileInfo_t *a = (FileInfo_t *)v1;
	FileInfo_t *b = (FileInfo_t *)v2;

	return simpleComp64((uint64)a->Stamp, (uint64)b->Stamp);
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
	char *file;
	FileInfo_t *i;
	FileInfo_t *i2;
	uint index;
	uint index2;

	foreach(fileInfos, i, index)
		addElement(files, (uint)i->File);

	editLines(files);
	errorCase_m(getCount(files) != getCount(fileInfos), "�ҏW�G���[�F�t�@�C���������������B");

	foreach(fileInfos, i, index)
	{
		i->EO_Index = findLine(files, i->File);
		errorCase_m(i->EO_Index == getCount(fileInfos), "�ҏW�G���[�F�t�@�C������");
	}
	foreach(fileInfos, i,  index)
	foreach(fileInfos, i2, index2)
	if(index < index2)
	{
		errorCase_m(i->EO_Index == i2->EO_Index, "�ҏW�G���[�F�t�@�C���d��");
	}
	rapidSort(fileInfos, CompEOIndex);

	releaseAutoList(files);
}
static void ChangeStamps(autoList_t *fileInfos)
{
	// TODO
}
static void DoOrderStampEdit(void)
{
	autoList_t *files = lsFiles(".");
	char *file;
	uint index;
	autoList_t *fileInfos = newList();

	LOGPOS();

	foreach(files, file, index)
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

	{
		FileInfo_t *i;

		foreach(fileInfos, i, index)
		{
			if(i->OldStamp != i->Stamp)
			{
				uint64 oldStamp = getFileStampByTime(i->OldStamp);
				uint64 stamp    = getFileStampByTime(i->Stamp);

				cout("%I64u -> %I64u : %s\n", oldStamp, stamp, i->File);
			}
		}
	}

	// Confirm
	{
		cout("�^�C���X�^���v���X�V���܂��B\n");
		cout("���s�H\n");

		if(clearGetKey() == 0x1b)
			termination(0);

		cout("���s���܂��B\n");
	}

	{
		FileInfo_t *i;

		foreach(fileInfos, i, index)
		{
			if(i->OldStamp != i->Stamp)
			{
				uint64 stamp = getFileStampByTime(i->Stamp);

				cout("%I64u -> %s\n", stamp, i->File);

				setFileStamp(i->File, stamp, stamp, stamp);
			}
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
