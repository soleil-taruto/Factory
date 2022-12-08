/*
	fileStore.exe にアップロードする。
		mmpc.exe /S upload upload /F file <アップロードするファイル又はフォルダ> _content.tmp
		hget.exe /C _content.tmp <鯖>:<港>
		DEL _content.tmp

		<アップロードするファイル又はフォルダ> を * にするとドラッグアンドドロップになる。
*/

#include "C:\Factory\Common\all.h"
#include "libs\http\MakeMultiPartContent.h"

int main(int argc, char **argv)
{
	char *text;
	char *bodyLine;
	char *bodyFile;
	char *bodyPath;
	char *fileName;
	autoBlock_t gab;
	autoBlock_t *content;

readArgs:
	if (argIs("/S")) // String
	{
		text = nextArg();
		bodyLine = nextArg();
		mmpc_addPart(text, "", gndBlockLineVar(bodyLine, gab));
		goto readArgs;
	}
	if (argIs("/B")) // Binary-file
	{
		text = nextArg();
		bodyFile = nextArg();
		mmpc_addPartFile(text, "", bodyFile);
		goto readArgs;
	}
	if (argIs("/F")) // File
	{
		text = nextArg();
		bodyPath = nextArg();

		if (bodyPath[0] == '*')
			bodyPath = dropDirFile();
		else
			bodyPath = makeFullPath(bodyPath);

		if (existDir(bodyPath)) // ? ファイルじゃなくてDIR -> clu化してファイルにする。
		{
			bodyFile = makeTempPath("clu");
			coExecute_x(xcout("C:\\Factory\\Tools\\Cluster.exe /I /M \"%s\" \"%s\"", bodyFile, bodyPath));
			fileName = xcout("%s.clu", bodyPath);

			mmpc_addPartFile(text, fileName, bodyFile);

			removeFile(bodyFile);
			memFree(bodyFile);
			memFree(fileName);
		}
		else
		{
			mmpc_addPartFile(text, bodyPath, bodyPath);
		}
		memFree(bodyPath);
		goto readArgs;
	}

	content = makeMultiPartContent();
	writeBinary(nextArg(), content);
	releaseAutoBlock(content);
}
