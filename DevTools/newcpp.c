/*
	newcpp g  �v���W�F�N�g��
	newcpp g2 �v���W�F�N�g��
	newcpp g3 �v���W�F�N�g��
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
	errorCase((uint)q - (uint)p != 38); // {} �t�� UUID �̒���

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

	errorCase_m(!lineExp("<1,30,__09AZaz>", project), "�s���ȃv���W�F�N�g���ł��B");
	errorCase_m(existPath(project), "���ɑ��݂��܂��B");

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

		removeFileIfExist("C:\\Factory\\tmp\\Sections.txt"); // �Ӑ}���Ȃ��������ʂ� trep ���Ȃ��悤�ɁA�O�̂��ߌ������ʂ��N���A

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
	cout("usage: newcpp (G�bG2�bG3�b?) �v���W�F�N�g��\n");
	cout("                      ��\n");
	cout("                     ���\n");
}
