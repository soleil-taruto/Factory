/*
	<Factory>\SubTools\KitDL.c �� httpGetOrPost() ver.
*/

#include "C:\Factory\Labo\Socket\libs\http\httpClient.h"

static char *DLUrl = "http://localhost/release/Kit";
static char *DestDir = "C:\\app\\Kit";

static autoList_t *AppNames;
static autoList_t *AppUrls;

int main(int argc, char **argv)
{
	autoBlock_t *indexFileImage;

readArgs:
	if (argIs("/U"))
	{
		DLUrl = nextArg();
		goto readArgs;
	}
	if (argIs("/D"))
	{
		DestDir = nextArg();
		goto readArgs;
	}
	errorCase(m_isEmpty(DLUrl));
	errorCase(!existDir(DestDir));

	AppNames = newList();
	AppUrls = newList();

	LOGPOS();

	indexFileImage = httpGetOrPost(DLUrl, NULL);

	LOGPOS();

	errorCase_m(!indexFileImage, "��M���s");

	// index �_�E�����[�h & ����
	{
		char *indexFile = makeTempPath("KitDL.index.tmp");
		autoList_t *lines;
		char *line;
		uint index;

		writeBinary_cx(indexFile, indexFileImage);

		lines = readLines(indexFile);

		foreach (lines, line, index)
		{
			if (startsWith(line, "<div><a href=\""))
			{
				char *p = strchr(line, '"') + 1;
				char *q;

				q = strchr(p, '"');

				if (q)
				{
					char *appFile;

					*q = '\0';
					appFile = strx(p);

					if (!_stricmp("zip", getExt(appFile)) && isFairLocalPath(appFile, 100))
					{
						char *appName = changeExt(appFile, "");

						cout("�A�v��: [%s]\n", appName);

						addElement(AppNames, (uint)strx(appName));
						addElement(AppUrls, (uint)xcout("%s/%s", DLUrl, appFile));

						memFree(appName);
					}
					memFree(appFile);
				}
			}
		}
		releaseDim(lines, 1);
		removeFile(indexFile);
		memFree(indexFile);
	}

	LOGPOS();

	// �_�E�����[�h & �W�J
	{
		char *appName;
		uint index;

		foreach (AppNames, appName, index)
		{
			char *appUrl = getLine(AppUrls, index);
			char *appFile = makeTempPath("KitDL.app.zip");
			char *wDir = combine(DestDir, appName);

			cout("< %s\n", appUrl);
			cout("# %s\n", appFile);
			cout("> %s\n", wDir);

			errorCase_m(!httpGetOrPostFile(appUrl, NULL, appFile), "�A�v����M���s");

			LOGPOS();

			coSleep(2000); // �����݂͂��ς� HGet ���폜�ł��Ȃ��ƌ��Ȃ̂ŁA�����҂B

			if (existDir(wDir))
				semiRemovePath(wDir);

			createDir(wDir);

			coExecute_x(xcout("C:\\Factory\\SubTools\\zip.exe /U \"%s\" \"%s\"", appFile, wDir));

			removeFile(appFile);
			memFree(appFile);
			memFree(wDir);
		}
	}

	LOGPOS();
}
