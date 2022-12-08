/*
	chkuuidcoll.exe ディレクトリ ...

	機能：UUIDの重複をチェックする。

		大文字小文字だけ違うUUID -> 違うと見なす。
*/

#include "C:\Factory\Common\all.h"

#define UUID_FMT "{99999999-9999-9999-9999-999999999999}"
#define SHARED_PTN "shared_uuid"
#define NUM_JOINT_CHR ':'
#define GLOBAL_SFX "@g"
#define IGNORE_SFX "@ign"

typedef struct UUID_st
{
	char *File;
	uint LineNo;
	char *UUID;
	int SharedFlag;
	int GlobalFlag;
	uint SharedNum; // 0 == 個数指定無し
	char *Project;
}
UUID_t;

static autoList_t *UUIDList;

static void Line2Fmt(char *line)
{
	char *p;

	for (p = line; *p; p++)
	{
		if (isMbc(p))
		{
			*p++ = 'K';
			*p = 'K';
		}
		else if (m_ishexadecimal(*p))
		{
			*p = '9';
		}
	}
}
static char *GetProject(char *file)
{
	char *path = file;
	char *rumDir;

	path = makeFullPath(file); // 2bs

	for (; ; )
	{
		path = getParent_x(path);

		if (isAbsRootDir(path))
			break;

		rumDir = addExt(strx(path), "rum");

		if (existDir(rumDir))
		{
			memFree(rumDir);
			break;
		}
		memFree(rumDir);
	}
	return path;
}
static void CollectUUIDByFile(char *file)
{
	autoList_t *lines = readLines(file);
	char *line;
	uint index;

	foreach (lines, line, index)
	{
		char *fmt = strx(line);
		char *p;

		Line2Fmt(fmt);

		for (p = fmt; p = strstr(p, UUID_FMT); p += strlen(UUID_FMT))
		{
			char *uuidP = line + ((uint)p - (uint)fmt);

			if (!startsWith(uuidP, UUID_FMT)) // ? not UUID_FMT
			{
				UUID_t *i = nb_(UUID_t);
				char *pp;

				i->File = strx(file);
				i->LineNo = index + 1;
				i->UUID = strxl(uuidP, strlen(UUID_FMT));
				i->SharedFlag = (int)(pp = mbs_strstr(uuidP + strlen(UUID_FMT), SHARED_PTN));
//				i->GlobalFlag = 0;
//				i->SharedNum = 0;
				i->Project = GetProject(file);

				if (pp)
				{
					pp += strlen(SHARED_PTN);

					if (startsWith(pp, GLOBAL_SFX)) // グローバル指定
					{
						i->GlobalFlag = 1;
					}
					else if (startsWith(pp, IGNORE_SFX)) // このUUIDの存在を無視する。
					{
						goto ignoreThisUUID; // gomi: i
					}
					else if (*pp == NUM_JOINT_CHR) // 個数指定
					{
						char *q;

						pp++;

						for (q = pp; *q; q++)
							if (!m_isdecimal(*q))
								break;

						errorCase(q == pp); // 個数が記述されていない。

						i->SharedNum = toValue_x(strxl(pp, (uint)q - (uint)pp));

						errorCase(i->SharedNum < 2); // 意味のない個数
					}
				}
				addElement(UUIDList, (uint)i);
			ignoreThisUUID:;
			}
		}
		memFree(fmt);
	}
	releaseDim(lines, 1);
}
static int IsTargetFile(char *file)
{
	char *ext = getExt(file);

	return
		!_stricmp(ext, "c") ||
		!_stricmp(ext, "h") ||
		!_stricmp(ext, "cs") ||
		!_stricmp(ext, "cpp") ||
		!_stricmp(ext, "java");
}
static void CollectUUIDByPath(char *path)
{
	if (existDir(path))
	{
		autoList_t *files = lssFiles(path);
		char *file;
		uint index;

		foreach (files, file, index)
			if (IsTargetFile(file))
				CollectUUIDByFile(file);

		releaseDim(files, 1);
	}
	else
	{
		CollectUUIDByFile(path);
	}
}
static void CollectUUID(void)
{
	UUIDList = newList();

	if (hasArgs(1))
	{
		while (hasArgs(1))
		{
			char *path = makeFullPath(nextArg());

			CollectUUIDByPath(path);
			memFree(path);
		}
	}
	else
	{
		CollectUUIDByPath(c_dropDirFile());
	}
}
static void DispAllProject(void)
{
	UUID_t *i;
	uint index;

	cout("---- Projects\n");

	foreach (UUIDList, i, index)
	{
		cout("[%s] %s (%u)\n", i->Project, i->File, i->LineNo);
	}
}

// ---- DispAllUUID ----

static sint CompUUIDByFile(uint i, uint j)
{
	return mbs_stricmp(((UUID_t *)i)->File, ((UUID_t *)j)->File);
}
static sint CompUUIDByUUID(uint i, uint j)
{
	sint ret = _stricmp(((UUID_t *)i)->UUID, ((UUID_t *)j)->UUID);

	if (ret)
		return ret;

	return mbs_stricmp(((UUID_t *)i)->File, ((UUID_t *)j)->File);
}
static void DispAllUUID_2(char *title)
{
	UUID_t *i;
	uint index;

	cout("---- %s\n", title);

	foreach (UUIDList, i, index)
	{
		char *sflg;

		if (!i->SharedFlag)
		{
			sflg = strx("-");
		}
		else if (i->GlobalFlag)
		{
			sflg = strx("G");
		}
		else if (!i->SharedNum)
		{
			sflg = strx("S");
		}
		else
		{
			sflg = xcout("%u", i->SharedNum);
		}

		cout("%s [%s] %s (%u)\n", i->UUID, sflg, i->File, i->LineNo);

		memFree(sflg);
	}
}
static void DispAllUUID()
{
	rapidSort(UUIDList, CompUUIDByFile);
	DispAllUUID_2("ASC FILE");
	rapidSort(UUIDList, CompUUIDByUUID);
	DispAllUUID_2("ASC UUID");
}

// ----

static autoList_t *FoundList;

static uint GetProjectCountUUID(char *target)
{
	UUID_t *i;
	uint index;
	autoList_t *prjs = newList();
	uint count;

	foreach (UUIDList, i, index)
		if (!_stricmp(i->UUID, target)) // 大文字小文字だけ違うUUID -> 同じと見なす。
			addElement(prjs, (uint)strx(i->Project));

	prjs = autoDistinctJLinesICase(prjs);
	count = getCount(prjs);
	releaseDim(prjs, 1);
	return count;
}
static uint GetUUIDCount(char *target)
{
	UUID_t *i;
	uint index;
	uint count = 0;

	foreach (UUIDList, i, index)
		if (!_stricmp(i->UUID, target)) // 大文字小文字だけ違うUUID -> 同じと見なす。
			count++;

	return count;
}
static void DispAllWarning(void)
{
	UUID_t *i;
	uint index;

	foreach (UUIDList, i, index)
	{
		if (i->SharedFlag && GetUUIDCount(i->UUID) < 2)
		{
			cout("----\n");
			cout("共有指定されているけど共有されていない。\n");
			cout("%s (%u)\n", i->File, i->LineNo);
		}
	}
}
static void DispAllUUIDError(void)
{
	UUID_t *i;
	UUID_t *j;
	uint index_i;
	uint index_j;

	foreach (UUIDList, i, index_i)
	foreach (UUIDList, j, index_j)
	{
		if (
			index_i != index_j &&
			!strcmp(i->UUID, j->UUID)
			)
		if (
			m_01(i->SharedFlag) != m_01(j->SharedFlag) ||
			m_01(i->GlobalFlag) != m_01(j->GlobalFlag) ||
			i->SharedNum != j->SharedNum
			)
		{
			cout("----\n");
			cout("【警告】同じUUID同士でパラメータが異なる。\n");
			cout("%s (%u) %c %c %u\n", i->File, i->LineNo, i->SharedFlag ? 'S' : '-', i->GlobalFlag ? 'G' : '-', i->SharedNum);
		}
	}
}
static void DispAllSharedNumError(void)
{
	UUID_t *i;
	uint index;

	foreach (UUIDList, i, index)
	{
		if (i->SharedFlag && i->SharedNum && i->SharedNum != GetUUIDCount(i->UUID))
		{
			cout("----\n");
			cout("【警告】個数が合わない。\n");
			cout("指定の個数＝%u\n", i->SharedNum);
			cout("実際の個数＝%u\n", GetUUIDCount(i->UUID));
			cout("%s (%u)\n", i->File, i->LineNo);

			addElement(FoundList, (uint)strx(i->File));
		}
	}
}
static void DispAllCollisionInterProject(void)
{
	UUID_t *i;
	uint index;

	foreach (UUIDList, i, index)
	{
		if (!i->GlobalFlag && 2 <= GetProjectCountUUID(i->UUID))
		{
			cout("----\n");
			cout("【警告】プロジェクト間で重複している。\n");
			cout("%s (%u)\n", i->File, i->LineNo);

			addElement(FoundList, (uint)strx(i->File));
		}
	}
}
static void DispAllCollision(void)
{
	UUID_t *i;
	uint index;

	foreach (UUIDList, i, index)
	{
		if (!i->SharedFlag && 2 <= GetUUIDCount(i->UUID))
		{
			cout("----\n");
			cout("【警告】重複している。\n");
			cout("%s (%u)\n", i->File, i->LineNo);

			addElement(FoundList, (uint)strx(i->File));
		}
	}
}

int main(int argc, char **argv)
{
	antiSubversion = 1;

	FoundList = newList();

	CollectUUID();
	DispAllProject();
	DispAllUUID();
	DispAllWarning();
	DispAllUUIDError();
	DispAllSharedNumError();
	DispAllCollisionInterProject();
	DispAllCollision();

	FoundList = autoDistinctJLinesICase(FoundList);

	writeLines(FOUNDLISTFILE, FoundList);
}
