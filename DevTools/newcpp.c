/*
	newcpp g  プロジェクト名
	newcpp g2 プロジェクト名
	newcpp g3 プロジェクト名
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\CRandom.h"

#define LOCAL_PROGRAM_CS "Program.cs"

static void RenamePaths(char *fromPtn, char *toPtn)
{
	autoList_t *paths = ls(".");
	char *path;
	char *nPath;
	uint dirCount;
	uint index;

	eraseParents(paths);
	dirCount = lastDirCount;

	foreach(paths, path, index)
	{
		if(index < dirCount)
		{
			addCwd(path);
			{
				RenamePaths(fromPtn, toPtn);
			}
			unaddCwd();
		}
		nPath = replaceLine(strx(path), fromPtn, toPtn, 1);

		if(replaceLine_getLastReplacedCount())
		{
			coExecute_x(xcout("REN \"%s\" \"%s\"", path, nPath));
		}
		memFree(nPath);
	}
	releaseDim(paths, 1);
}
static void ChangeAppIdent(char *srcFile)
{
	char *src = readText(srcFile);
	char *p;
	char *q;
	char *uuid;

	p = strstrNext(src, "public const string APP_IDENT = \"");
	errorCase(!p);
	q = strstr(p, "\";");
	errorCase(!q);
	errorCase((uint)q - (uint)p != 38); // {} 付き UUID の長さ

	uuid = MakeUUID(1);

	memcpy(p, uuid, 38);

	writeOneLineNoRet(srcFile, src);

	memFree(src);
	memFree(uuid);
}
static void Main2(char *tmplProject, char *tmplDir)
{
	char *project = nextArg();

	errorCase(!existDir(tmplDir));

	errorCase_m(!lineExp("<1,30,__09AZaz>", project), "不正なプロジェクト名です。");
	errorCase_m(existPath(project), "既に存在します。");

	createDir(project);
	copyDir(tmplDir, project);

	addCwd(project);
	{
		coExecute("qq -f");

		RenamePaths(tmplProject, project);

		addCwd(project);
		{
			addCwd(project);
			{
				if(existFile(LOCAL_PROGRAM_CS))
					ChangeAppIdent(LOCAL_PROGRAM_CS);
			}
			unaddCwd();
		}
		unaddCwd();

		removeFileIfExist("C:\\Factory\\tmp\\Sections.txt"); // 意図しない検索結果を trep しないように、念のため検索結果をクリア

		coExecute_x(xcout("Search.exe %s", tmplProject));
		coExecute_x(xcout("trep.exe /F %s", project));

//		coExecute("runsub _copylib");

		execute("START .");
	}
	unaddCwd();
}
int main(int argc, char **argv)
{
	if(argIs("G"))
	{
		Main2("GGGGTMPL", "C:\\Dev\\Annex\\Template\\GameTemplate");
		return;
	}
	if(argIs("G2"))
	{
		Main2("GGGGTMPL", "C:\\Dev\\Annex\\Template\\GameTemplate2");
		return;
	}
	if(argIs("G3"))
	{
		Main2("GGGGTMPL", "C:\\Dev\\Annex\\Template\\GameTemplate3");
		return;
	}
	cout("usage: newcpp (G｜G2｜G3｜?) プロジェクト名\n");
	cout("                      ↑\n");
	cout("                     定番\n");
}
