/*
	mimiko 専用 KitDL
*/

#include "C:\Factory\Common\all.h"

static char *DLDir = "C:\\BlueFish\\BlueFish\\HTT\\stackprobe\\home\\_kit";
static char *DestDir = "C:\\app\\Kit";

static autoList_t *AppNames;
static autoList_t *AppUrls;

int main(int argc, char **argv)
{
readArgs:
	if (argIs("/U"))
	{
		DLDir = nextArg();
		goto readArgs;
	}
	if (argIs("/D"))
	{
		DestDir = nextArg();
		goto readArgs;
	}
	errorCase(!existDir(DLDir));
	errorCase(!existDir(DestDir));

	AppNames = newList();
	AppUrls = newList();

	LOGPOS();

#if 0 // old
	HGet_Reset();
	HGet_SetUrl(DLUrl);
	errorCase_m(!HGet_GetOrPost(), "受信失敗");
#endif

	LOGPOS();

	// index ダウンロード & 処理
	{
		char *indexFile = makeTempPath("KitDL.index.tmp");
		autoList_t *lines;
		char *line;
		uint index;

		copyFile_xc(combine(DLDir, "index.html"), indexFile);
//		HGet_MvResBodyFile(indexFile); // old

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

						cout("アプリ: [%s]\n", appName);

						addElement(AppNames, (uint)strx(appName));

						addElement(AppUrls, (uint)xcout("%s", appFile));
//						addElement(AppUrls, (uint)xcout("%s/%s", DLUrl, appFile)); // old

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

	// ダウンロード & 展開
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

#if 1
			copyFile_xc(combine(DLDir, appUrl), appFile);
#else // old
			HGet_Reset();
			HGet_SetUrl(appUrl);
			errorCase_m(!HGet_GetOrPost(), "アプリ受信失敗");
			HGet_MvResBodyFile(appFile);
			HGet_Reset(); // HGet を触るので、一旦解放
#endif

			LOGPOS();

			coSleep(2000); // 何か掴みっぱで HGet を削除できないと嫌なので、少し待つ。

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

//	HGet_Reset(); // 解放 // old
}
