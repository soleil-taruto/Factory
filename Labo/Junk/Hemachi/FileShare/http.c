#include "C:\Factory\Common\Options\SockServerTh.h"
#include "C:\Factory\Labo\Socket\libs\http\GetPost.h"
#include "libs\all.h"

#define HTTP_NEWLINE "\r\n"
#define VIR_PATH_DLMTR '$'

static autoList_t *DistinctJLines_x(autoList_t *lines)
{
	autoList_t *newLines = newList();
	autoList_t *others = newList();

	distinctJLinesICase(lines, newLines, others);

	releaseAutoList(lines);
	releaseDim(others, 1);
	return newLines;
}
static char *MakeUIPath(char *pathBase)
{
	char *result = xcout("%c%s%c", VIR_PATH_DLMTR, pathBase, VIR_PATH_DLMTR);

	trimSequ(result, VIR_PATH_DLMTR);
	replaceChar(result, VIR_PATH_DLMTR, '/');
	return result;
}
static char *MakeEncParent(char *pathBase)
{
	autoList_t *tokens = tokenize(pathBase, VIR_PATH_DLMTR);
	char *result;

	trimLines(tokens);

	if (getCount(tokens))
		memFree((char *)unaddElement(tokens));

	pathBase = untokenize_xx(tokens, xcout("%c", VIR_PATH_DLMTR));

	if (*pathBase)
		pathBase = addChar(pathBase, VIR_PATH_DLMTR);

	result = httpUrlEncoder(pathBase);
	memFree(pathBase);
	return result;
}
static void MakeEncUrlLabels(char *pathBase, autoList_t *subPaths, int dirMode, autoList_t **p_encUrls, autoList_t **p_labels, autoList_t **p_exts)
{
	char *encPathBase = httpUrlEncoder(pathBase);
	char *subPath;
	uint index;
	char *encSubPath;

	*p_encUrls = newList();
	*p_labels = newList();

	if (p_exts)
		*p_exts = newList();

	foreach (subPaths, subPath, index)
	{
		subPath = strx(subPath);

		if (*subPath == '\0')
			subPath = addLine(subPath, "----");

		{
			char *subPath2 = strx(subPath);

			if (dirMode)
				subPath2 = addChar(subPath2, VIR_PATH_DLMTR);

			encSubPath = httpUrlEncoder(subPath2);
			memFree(subPath2);
		}

		addElement(*p_encUrls, (uint)xcout("%s%s", encPathBase, encSubPath));
		addElement(*p_labels, (uint)strx(subPath));

		if (p_exts)
			addElement(*p_exts, (uint)strx(getExt(subPath)));

		memFree(subPath);
		memFree(encSubPath);
	}
	memFree(encPathBase);
}
static autoBlock_t *MakeIndexBody(
	char *uiPath,
	char *encParent,
	autoList_t *encDirUrls,
	autoList_t *dirLabels,
	autoList_t *encFileUrls,
	autoList_t *fileLabels,
	autoList_t *encFileExts,
	autoList_t *realFiles
	)
{
	autoBlock_t *result = newBlock();
	char *url;
	uint index;
	uint imgcnt = 0;
	uint64 imgszcnt = 0;

	ab_addLine(
		result,
		"<html>" HTTP_NEWLINE
		"<head>" HTTP_NEWLINE
		"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=Shift_JIS\"/>" HTTP_NEWLINE
		);

	ab_addLine_x(result, xcout("<title>hemachi fileShare - %s</title>" HTTP_NEWLINE, uiPath));

	ab_addLine(
		result,
		"<style type=\"text/css\">" HTTP_NEWLINE
		"<!--" HTTP_NEWLINE
		"a { text-decoration: none; }" HTTP_NEWLINE
		"a:hover { text-decoration: underline; }" HTTP_NEWLINE
		"a:link { color: ddeeff; }" HTTP_NEWLINE
		"a:visited { color: aabbcc; }" HTTP_NEWLINE
		"body { color: bbddff; background-color: 081018; }" HTTP_NEWLINE
		"div { font-family: Tahoma; font-size: x-small; margin: 1em; white-space: nowrap; }" HTTP_NEWLINE
		"<html>" HTTP_NEWLINE
		"-->" HTTP_NEWLINE
		"</style>" HTTP_NEWLINE
		"</head>" HTTP_NEWLINE
		"<body>" HTTP_NEWLINE
		);

	ab_addLine_x(result, xcout("<div>%s</div>" HTTP_NEWLINE, uiPath));

	if (strcmp(uiPath, "/") != 0) // ? not "/"
		ab_addLine_x(result, xcout("<div><a href=\"%s\">(PARENT)</a></div>" HTTP_NEWLINE, encParent));

	foreach (encDirUrls, url, index)
	{
		ab_addLine_x(result, xcout("<div><a href=\"%s\">%s</a>　(DIRECTORY)</div>" HTTP_NEWLINE, url, getLine(dirLabels, index)));
	}
	foreach (encFileUrls, url, index)
	{
		char *ext = getLine(encFileExts, index);

		ab_addLine_x(result, xcout("<div><a href=\"%s\">%s</a>" HTTP_NEWLINE, url, getLine(fileLabels, index)));

		if (
			!_stricmp("BMP", ext) ||
			!_stricmp("GIF", ext) ||
			!_stricmp("JPG", ext) ||
			!_stricmp("JPEG", ext) ||
			!_stricmp("PNG", ext)
			)
		{
			const uint IMGMAX = 50;
			const uint IMGSZMAX = 50 * 1024 * 1024;

			if (IMGMAX <= imgcnt)
			{
				ab_addLine_x(result, xcout("　(over %u images)" HTTP_NEWLINE, IMGMAX));
			}
			else if (IMGSZMAX <= imgszcnt)
			{
				ab_addLine_x(result, xcout("　(over %u bytes)" HTTP_NEWLINE, IMGSZMAX));
			}
			else
			{
				ab_addLine_x(result, xcout("<br/><img src=\"%s\"/>" HTTP_NEWLINE, url));

				imgcnt++;
				imgszcnt += getFileSize(getLine(realFiles, index));
			}
		}
		ab_addLine(result, "</div>");
	}

	ab_addLine(
		result,
		"</body>" HTTP_NEWLINE
		"</html>" HTTP_NEWLINE
		);

	return result;
}

static int DoLockPath(char *path)
{
	uint count;

	for (count = 60; ; count--)
	{
		cout("DoLockPath_count: %u\n", count);

		if (HFS_LockPath(path, 0))
			return 1;

		if (!count)
			break;

		inner_uncritical();
		{
			Sleep(1000);
		}
		inner_critical();
	}
	cout("DoLockPath_fault!\n");
	return 0;
}
static void PerformTh(int sock, char *strip)
{
	SockStream_t *i = CreateSockStream(sock, 90000); // 86400 + margin
	char *header;
	uchar *content;
	uint contentSize;
	httpDecode_t dec;
	char *dlFile;

	httpRecvRequest(i, &header, &content, &contentSize);
	httpDecode(header, content, &dec);

	dlFile = strx(dec.DecPath);

	memFree(header);
	memFree(content);
	httpDecodeFree(&dec);

	line2JLine(dlFile, 1, 0, 0, 1);
	cout("dlFile_1: %s\n", dlFile);

	if (*dlFile)
		dlFile = lineToFairLocalPath_x(dlFile, strlen(HFS_StoreDir));

	cout("dlFile_2: %s\n", dlFile);

	HFS_MutexEnter();
	{
		addCwd(HFS_StoreDir);

		if (*dlFile && existFile(dlFile))
		{
			char *fp_dlFile = combine(HFS_StoreDir, dlFile);

			HFS_MutexLeave(); // unlock
			{
				if (DoLockPath(fp_dlFile))
				{
					httpSendResponseFile(i, dlFile);
					HFS_UnlockPath(fp_dlFile);
				}
			}
			HFS_MutexEnter(); // lock

			memFree(fp_dlFile);
		}
		else
		{
			autoList_t *files = ls("."); // ファイルのみ、サブディレクトリを想定しない。
			char *file;
			uint index;
			autoList_t *subDirs = newList();
			autoList_t *subFiles = newList();
			autoList_t *realFiles = newList();

			eraseParents(files);

			foreach (files, file, index)
			{
				if (mbsStartsWithICase(file, dlFile))
				{
					char *lclFile = file + strlen(dlFile);
					char *p;

					p = strchr(lclFile, VIR_PATH_DLMTR);

					if (p)
					{
						*p = '\0';
						addElement(subDirs, (uint)strx(lclFile));
					}
					else
					{
						addElement(subFiles, (uint)strx(lclFile));
						addElement(realFiles, (uint)strx(file));
					}
				}
			}
			releaseDim(files, 1);

			sortJLinesICase(subDirs);
			sortJLinesICase(subFiles);

			subDirs = DistinctJLines_x(subDirs);

			{
				char *uiPath = MakeUIPath(dlFile);
				char *encParent = MakeEncParent(dlFile);
				autoList_t *encDirUrls;
				autoList_t *dirLabels;
				autoList_t *encFileUrls;
				autoList_t *fileLabels;
				autoList_t *encFileExts;

				MakeEncUrlLabels(dlFile, subDirs, 1, &encDirUrls, &dirLabels, NULL);
				MakeEncUrlLabels(dlFile, subFiles, 0, &encFileUrls, &fileLabels, &encFileExts);

				{
					autoBlock_t *body = MakeIndexBody(uiPath, encParent, encDirUrls, dirLabels, encFileUrls, fileLabels, encFileExts, realFiles);

					HFS_MutexLeave(); // unlock
					{
						httpSendResponse(i, body, "text/html; charset=Shift_JIS");
					}
					HFS_MutexEnter(); // lock

					releaseAutoBlock(body);
				}

				memFree(uiPath);
				memFree(encParent);
				releaseDim(encDirUrls, 1);
				releaseDim(dirLabels, 1);
				releaseDim(encFileUrls, 1);
				releaseDim(fileLabels, 1);
				releaseDim(encFileExts, 1);
			}

			releaseDim(subDirs, 1);
			releaseDim(subFiles, 1);
			releaseDim(realFiles, 1);
		}
		unaddCwd();
	}
	HFS_MutexLeave();

	ReleaseSockStream(i);
	memFree(dlFile);
}
static int IdleTh(void)
{
	while (hasKey())
	{
		if (getKey() == 0x1b)
		{
			return 0;
		}
		cout("ESC to EXIT.\n");
	}
	return 1;
}
int main(int argc, char **argv)
{
	uint portno = 80;

	if (argIs("/P"))
	{
		portno = toValue(nextArg());
	}

	cmdTitle("Hemachi FileShare http Server");

	HFS_StoreDir = nextArg();
	HFS_StoreDir = makeFullPath(HFS_StoreDir);
	cout("HFS_StoreDir: %s\n", HFS_StoreDir);

	errorCase(!existDir(HFS_StoreDir));
	errorCase(!mbs_stricmp("C:\\", HFS_StoreDir)); // 2bs

	sockServerTh(PerformTh, portno, 10, IdleTh);
}
