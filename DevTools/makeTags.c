/*
	秀丸設定memo

		ファイルタイプ別の設定 / その他 / 単語補完

			自動表示 [レ]
			表示方法 : リスト
			現在編集中のテキスト [レ]
			辞書ファイル [　]

			単語補完の検索対象 / 詳細

				[○] 行単位
				[レ] tagsファイル
				[　] ファイル名

		動作環境 / その他のコマンド / タグジャンプ / 上の階層もチェックする [レ]

	----

	makeTags.exe 対象ルートDIR

		対象ルートDIRとその配下にあるC言語のソースのタグ情報を <対象ルートDIR>\tags に出力する。

		対象となるファイル：
			*.c
			*.h (*.cpp のあるフォルダを除外)

	makeTags.exe /JS 入力JSファイル 出力tagsファイル

		入力JSファイルのタグ情報を出力tagsファイルに出力する。
*/

#include "C:\Factory\Common\all.h"

static autoList_t *Tags;

static void AddTag(char *symbol, char *comment, char *srcFile, uint srcLineNo)
{
	char *tag = xcout("%s(%u) : %s // %s", srcFile, srcLineNo, symbol, comment);

	cout("ADD: %s\n", tag);
	addElement(Tags, (uint)tag);
}
static void RemoveLiteralString(char *entity)
{
	char *p = entity;
	char *q;

	while(*p)
	{
		if(*p == '"')
		{
			p++;
			break;
		}
		if(*p == '\\' && p[1])
			p += 2;
		else
			p = mbsNext(p);
	}
	q = p;

	for(p = entity - 1; p < q; p++)
		*p = 'L';
}
static void RemoveComments(autoList_t *lines)
{
	char *line;
	uint index;
	int onComment = 0;

	foreach(lines, line, index)
	{
		char *p = line;

		if(!onComment)
		{
			char *q = strstr(p, "//");

			if(q)
				*q = '\0';
		}
		for(; ; )
		{
			char *q = strstr(p, onComment ? "*/" : "/*");

			if(!q)
				break;

			if(onComment)
			{
				copyLine(p, q + 2);
			}
			else
			{
				char *qq = strchr(p, '"');

				if(qq && qq < q)
				{
					RemoveLiteralString(qq + 1);
					continue;
				}
				p = q;
			}
			onComment = !onComment;
		}
		if(onComment)
			*p = '\0';
	}
}
static void AdjustIndent(autoList_t *lines)
{
	char *line;
	uint index;
	int onBlock = 0;

	foreach(lines, line, index)
	{
		if(!strcmp(line, "{"))
		{
			onBlock = 1;
		}
		else if(
			!strcmp(line, "}") ||
			!strcmp(line, "};")
			)
		{
			onBlock = 0;
		}
		else if(
			onBlock &&
			line[0] != ' ' &&
			line[0] != '#'
			)
		{
			line = insertChar(line, 0, ' ');
			setElement(lines, index, (uint)line);
		}
	}
}
static void CheckTagLine(char *line, char *srcFile, uint srcLineNo)
{
	char *tmpl = strx(line);
	char *p;
	autoList_t *tokens;

	tmpl = strx(line);

	for(p = tmpl; *p; p++)
		if(!__iscsym(*p))
			*p = ' ';

	tokens = ucTokenize(tmpl);

	if(lineExp("typedef<1,  ><>", line))
	{
		1; // noop
	}
	else if(lineExp("#define<1,  ><>", line))
	{
		if(2 <= getCount(tokens))
		{
			AddTag(getLine(tokens, 1), "定義", srcFile, srcLineNo);
		}
	}
	else if(__iscsymf(line[0]))
	{
		uint index = 0;

		// 記憶クラスをスキップする。
		{
			char *token = getLine(tokens, index);

			if(
				!strcmp(token, "static") ||
				!strcmp(token, "extern")
				)
				index++;
		}

		index++; // 評価される型をスキップする。

		if(index < getCount(tokens))
		{
			char *comment = "関数";

			if(lineExp("<>;", line))
				comment = "変数又は宣言";

			AddTag(getLine(tokens, index), comment, srcFile, srcLineNo);
		}
	}
	memFree(tmpl);
	releaseDim(tokens, 1);
}
static void CheckTagTypedef(autoList_t *lines, char *srcFile)
{
	char *line;
	uint index;
	uint tdPhase = 0;

	foreach(lines, line, index)
	{
		switch(tdPhase)
		{
		case 0:
			if(lineExp("typedef<1,  ><>", line) && !lineExp("<>;", line)) tdPhase++;
			break;

		case 1:
			if(!strcmp(line, "{")) tdPhase++;
			break;

		case 2:
			if(!strcmp(line, "}")) tdPhase++;
			break;

		case 3:
			{
				char *typnm = strx(line);
				char *p;

				for(p = typnm; *p; p++)
					if(!__iscsym(*p))
						*p = ' ';

				trim(typnm, ' ');
				AddTag(typnm, "型宣言", srcFile, index + 1);
				memFree(typnm);
			}
			tdPhase = 0;
			break;

		default:
			error();
		}
	}
}
static void FindTagsByFile(char *file)
{
	autoList_t *lines = readLines(file);
	char *line;
	uint index;

	RemoveComments(lines);
	ucTrimSqTrailAllLine(lines);
	AdjustIndent(lines);

	foreach(lines, line, index)
	{
		CheckTagLine(line, file, index + 1);
	}
	CheckTagTypedef(lines, file);

	releaseDim(lines, 1);
}

#define IGNORE_JS_NAME_PREFIX "@@_"

static void FindTagsByJSFile(char *file)
{
	autoList_t *lines = readLines(file);
	char *line;
	uint index;

	RemoveComments(lines);
	ucTrimSqTrailAllLine(lines);

	foreach(lines, line, index)
	{
		if(lineExp("var <>;<>", line))
		{
			char *p = ne_strchr(line, ' ') + 1;
			char *q;
			char *q2;

			q = ne_strchr(p, ';');
			q2 = strchr(p, '=');

			if(q2)
				q = m_min(q, q2);

			*q = '\0';
			trimTrail(p, ' ');

			if(!startsWith(p, IGNORE_JS_NAME_PREFIX))
			{
				AddTag(p, "変数", file, index + 1);
			}
		}
		else if(lineExp("function <>(<>", line))
		{
			char *p = ne_strchr(line, ' ') + 1;
			char *q;

			q = ne_strchr(p, '(');
			*q = '\0';

			if(!startsWith(p, IGNORE_JS_NAME_PREFIX))
			{
				AddTag(p, "関数", file, index + 1);
			}
		}
	}
	releaseDim(lines, 1);
}
static void FindTags(char *rootDir)
{
	autoList_t *files = lssFiles(rootDir);
	char *file;
	uint index;

	sortJLinesICase(files);

	foreach(files, file, index)
	{
		char *ext = getExt(file);

		if(
			!_stricmp(ext, "c") ||
			!_stricmp(ext, "h") &&
//			!existFile(c_changeExt(file, "cpp")) // .h のみとかある。
			/*
			!existFile(c_changeLocal(file, "Main.cpp")) &&
			!existFile(c_changeLocal(file, "_Main.cpp")) &&
			!existFile(c_changeLocal(file, "AAMain.cpp"))
			*/
			!fileSearchExist(c_changeLocal(file, "*.cpp"))
			)
			FindTagsByFile(file);
	}
	releaseDim(files, 1);
}
static void MakeTags(char *rootDir)
{
	char *tagsFile;

	errorCase(!existDir(rootDir));
	rootDir = makeFullPath(rootDir);

	Tags = newList();

	FindTags(rootDir);

	tagsFile = combine(rootDir, "tags");

	writeLines(tagsFile, Tags);

	releaseDim(Tags, 1);
	memFree(tagsFile);
}
static void MakeJSTags(char *jsFile, char *tagsFile)
{
	jsFile = makeFullPath(jsFile);

	Tags = newList();

	FindTagsByJSFile(jsFile);

	writeLines(tagsFile, Tags);

	memFree(jsFile);
}
#if 0 // old
static void MakeJSTags_Dir(char *dir)
{
	char *jsFile   = combine(dir, "module.js");
	char *tagsFile = combine(dir, "tags");

	MakeJSTags(jsFile, tagsFile);

	memFree(jsFile);
	memFree(tagsFile);
}
#endif
int main(int argc, char **argv)
{
	if(argIs("/JS"))
	{
#if 1
		MakeJSTags(getArg(0), getArg(1));
#else // old
		if(hasArgs(2))
		{
			MakeJSTags(getArg(0), getArg(1));
		}
		else
		{
			MakeJSTags_Dir(nextArg());
		}
#endif
	}
	else
	{
		MakeTags(hasArgs(1) ? nextArg() : c_dropDir());
	}
}
