#include "GitResourceMask.h"

#define FLAG_FILE "_gitresmsk"

#define WAV_HZ 8000
//#define WAV_HZ 44100
#define MOVIE_FPS 30

#define CACHE_DIR "C:\\Factory\\tmp\\GitResourceMaskCache"

static char *GetCachedFile(char *categoly, char *keyFile)
{
	static char *file;

	memFree(file);

	createDirIfNotExist(CACHE_DIR);
	file = combine_cx(CACHE_DIR, xcout("%s_%s.dat", categoly, c_md5_makeHexHashFile(keyFile)));
	cout("cached_file: %s\n", file);
	return file;
}
static TouchFile(char *file)
{
	uint64 stamp = getFileStampByTime(0);

	setFileStamp(file, 0ui64, stamp, stamp);
}

typedef struct MediaInfo_st
{
	uint Centisec;
	uint Width;
	uint Height;
}
MediaInfo_t;

static MediaInfo_t GetMediaInfo(char *file)
{
	char *repFile = makeTempPath(NULL);
	autoList_t *lines;
	char *line;
	uint index;
	MediaInfo_t ret = { 0 };

	coExecute_x(xcout("START \"\" /B /WAIT \"%s\" \"%s\" 2> \"%s\"", FILE_FFPROBE_EXE, file, repFile));

	lines = readLines(repFile);

	foreach (lines, line, index)
	{
		char *p;
		char *q;

		ucTrim(line);

		// sample:
		// ... Duration: 00:01:12.96, start: 0.000000, ...
		// Stream #0:0(und): Video: h264 ... v420p, 512x384, 361 kb/s, ...

		if (p = strstrNext(line, "Duration: "))
		{
			p = toknext(p, " ,");
			errorCase(!p);

			errorCase(!lineExp("<2,09>:<2,09>:<2,09>.<2,09>", p));

			p[2] = '\0';
			p[5] = '\0';
			p[8] = '\0';

			ret.Centisec =
				toValue(p) * 360000 +
				toValue(p + 3) * 6000 +
				toValue(p + 6) * 100 +
				toValue(p + 9);

			errorCase(!m_isRange(ret.Centisec, 1, IMAX));
		}
		else if (startsWith(line, "Stream") && (p = strstrNext(line, "Video:")))
		{
			tokinit(p, " ,");

			while (p = toknext(NULL, NULL))
			{
				if (lineExp("<1,,09>x<1,,09>", p))
				{
					ret.Width  = toValue(toknext(p, "x"));
					ret.Height = toValue(toknext(NULL, ""));

					errorCase(!m_isRange(ret.Width,  1, IMAX));
					errorCase(!m_isRange(ret.Height, 1, IMAX));

					break;
				}
			}
		}
	}
	releaseDim(lines, 1);
	removeFile(repFile);
	memFree(repFile);
	return ret;
}

// ---- Image ----

static void MaskResImage_Main(char *file)
{
	char *midFile = makeTempPath("png");

	coExecute_x(xcout("START \"\" /B /WAIT \"%s\" /MASK-RESOURCE-IMAGE-NBC \"%s\" \"%s\"", FILE_TOOLKIT_EXE, file, midFile));

	if (!_stricmp("png", getExt(file)))
	{
		removeFile(file);
		moveFile(midFile, file);
		createFile(midFile);
	}
	else
	{
		coExecute_x(xcout("START \"\" /B /WAIT \"%s\" /IMG-TO-IMG \"%s\" \"%s\" 0", FILE_TOOLKIT_EXE, midFile, file));
//		coExecute_x(xcout("START \"\" /B /WAIT \"%s\" /IMG-TO-IMG \"%s\" \"%s\"", FILE_TOOLKIT_EXE, midFile, file));
//		coExecute_x(xcout("START \"\" /B /WAIT \"%s\" \"%s\" \"%s\"", FILE_BMPTOCSV_EXE, midFile, file));
	}
	removeFile(midFile);
	memFree(midFile);
}
static void MaskResImage(char *file)
{
	char *cachedFile = GetCachedFile("image", file);

	if (existFile(cachedFile))
	{
		copyFile(cachedFile, file);
		TouchFile(cachedFile);
	}
	else
	{
		LOGPOS();
		MaskResImage_Main(file);
		LOGPOS();
		copyFile(file, cachedFile);
	}
}

// ---- Sound ----

static uint MRS_GetSoundLength(char *file)
{
	MediaInfo_t mi = GetMediaInfo(file);

	errorCase(!mi.Centisec); // ? no data

	m_range(mi.Centisec, 1, 60000); // 0s <, <= 10m

	return (uint)d2i(mi.Centisec / 100.0 * WAV_HZ);
}
static void MRS_MakeWavFile(char *file, uint length)
{
	char *csvFile = makeTempPath(NULL);
	FILE *fp;
	uint count;

	LOGPOS();
	fp = fileOpen(csvFile, "wt");

	for (count = 0; count < length; count++)
	{
		writeLine(fp, "32768,32768"); // 無音
	}
	fileClose(fp);
	LOGPOS();

	writeWAVFileFromCSVFile(csvFile, file, WAV_HZ);
	LOGPOS();

	removeFile(csvFile);
	memFree(csvFile);
}
static void MRS_MakeSoundFile(char *rFile, char *wFile)
{
	if (!_stricmp("wav", getExt(wFile)))
	{
		moveFile(rFile, wFile);
		createFile(rFile);
	}
	else
	{
		coExecute_x(xcout("START \"\" /B /WAIT \"%s\" -i \"%s\" \"%s\"", FILE_FFMPEG_EXE, rFile, wFile));
	}
}
static void MaskResSound_Main(char *file)
{
	char *tmpFile = makeTempPath(getExt(file));
	char *wavFile = makeTempPath("wav");
	char *outFile = makeTempPath(getExt(file));

	moveFile(file, tmpFile);

	MRS_MakeWavFile(wavFile, MRS_GetSoundLength(tmpFile));
	MRS_MakeSoundFile(wavFile, outFile);

	moveFile(outFile, file);

	removeFile(tmpFile);
	removeFile(wavFile);
	memFree(tmpFile);
	memFree(wavFile);
	memFree(outFile);
}
static void MaskResSound(char *file)
{
	char *cachedFile = GetCachedFile("sound", file);

	if (existFile(cachedFile))
	{
		copyFile(cachedFile, file);
		TouchFile(cachedFile);
	}
	else
	{
		LOGPOS();
		MaskResSound_Main(file);
		LOGPOS();
		copyFile(file, cachedFile);
	}
}

// ---- Movie ----

static char *MRM_GetKomaFile(char *dir, uint index)
{
	return combine_cx(dir, xcout("%u.png", index));
}
static void MRM_GenKoma(char *komaDir, char *file)
{
	MediaInfo_t mi = GetMediaInfo(file);
	uint length;
	uint index;
	char *komaFile_0 = MRM_GetKomaFile(komaDir, 0);
	char *komaFile_1 = MRM_GetKomaFile(komaDir, 1);

	// ? no data
	errorCase(!mi.Centisec);
	errorCase(!mi.Width);
	errorCase(!mi.Height);

	m_range(mi.Centisec, 300, 60000); // 3s ～ 10m
	m_range(mi.Width,  100, 1000);
	m_range(mi.Height, 100, 1000);

	length = d2i(mi.Centisec / 100.0 * MOVIE_FPS);

	coExecute_x(xcout("START \"\" /B /WAIT \"%s\" /WF \"%s\" /PLAIN %u %u", FILE_IMGTOOLS_EXE, komaFile_0, mi.Width, mi.Height));
	coExecute_x(xcout("START \"\" /B /WAIT \"%s\" /MASK-RESOURCE-IMAGE \"%s\" \"%s\" \"%s\"", FILE_TOOLKIT_EXE, komaFile_0, komaFile_1, file));

	removeFile(komaFile_0);
	moveFile(komaFile_1, komaFile_0);

	for (index = 1; index < length; index++)
	{
		char *komaFile_index = MRM_GetKomaFile(komaDir, index);

		copyFile(komaFile_0, komaFile_index);

		memFree(komaFile_index);
	}
	memFree(komaFile_0);
	memFree(komaFile_1);
}
static void MaskResMovie_Main(char *file)
{
	char *komaDir = makeTempDir(NULL);
	char *midFile;

	midFile = combine_cx(komaDir, xcout("1.%s", getExt(file)));

	MRM_GenKoma(komaDir, file);

	addCwd(komaDir);
	{
		coExecute_x(xcout("START \"\" /B /WAIT \"%s\" -r %u -i %%d.png %s", FILE_FFMPEG_EXE, MOVIE_FPS, getLocal(midFile)));
	}
	unaddCwd();

	removeFile(file);
	moveFile(midFile, file);

	recurRemoveDir(komaDir);
	memFree(komaDir);
	memFree(midFile);
}
static void MaskResMovie(char *file)
{
	char *cachedFile = GetCachedFile("movie", file);

	if (existFile(cachedFile))
	{
		copyFile(cachedFile, file);
		TouchFile(cachedFile);
	}
	else
	{
		LOGPOS();
		MaskResMovie_Main(file);
		LOGPOS();
		copyFile(file, cachedFile);
	}
}

// ---- Other ----

static void MaskResOther(char *file)
{
	char *hash = md5_makeHexHashFile(file);

	writeOneLine_cx(file, xcout("//// dummy data md5:%s ////", hash));
	memFree(hash);

	EscapeUnusableResPath(file);
}

// ----

static void MaskResourceFile(char *file)
{
	char *ext = getExt(file);

	cout("* %s\n", file);

	     if (!_stricmp(ext, "bmp"  )) MaskResImage(file);
	else if (!_stricmp(ext, "gif"  )) MaskResImage(file);
	else if (!_stricmp(ext, "jpeg" )) MaskResImage(file);
	else if (!_stricmp(ext, "jpg"  )) MaskResImage(file);
	else if (!_stricmp(ext, "png"  )) MaskResImage(file);
	//--
	else if (!_stricmp(ext, "mp3"  )) MaskResSound(file);
	else if (!_stricmp(ext, "wav"  )) MaskResSound(file);
	//--
	else if (!_stricmp(ext, "mp4"  )) MaskResMovie(file);
	else if (!_stricmp(ext, "mpeg" )) MaskResMovie(file);
	else if (!_stricmp(ext, "mpg"  )) MaskResMovie(file);
	//--
	else if (!_stricmp(ext, "avi"  )) MaskResOther(file); // movie
	else if (!_stricmp(ext, "csv"  )) MaskResOther(file);
	else if (!_stricmp(ext, "ogg"  )) MaskResOther(file); // audio
	else if (!_stricmp(ext, "ogv"  )) MaskResOther(file); // movie
	else if (!_stricmp(ext, "otf"  )) MaskResOther(file); // open-type font
	else if (!_stricmp(ext, "ttf"  )) MaskResOther(file); // true-type font
	else
		return;

	PostGitMaskFile(file);
}
void GitResourceMask(char *rootDir)
{
	autoList_t *files = lssFiles(rootDir);
	char *file;
	uint index;
	autoList_t *targets = newList();

	LOGPOS();

	// 外部コマンド存在確認
	{
		errorCase(!existFile(FILE_FFMPEG_EXE));
		errorCase(!existFile(FILE_FFPROBE_EXE));
//		errorCase(!existFile(FILE_BMPTOCSV_EXE));
		errorCase(!existFile(FILE_IMGTOOLS_EXE));
		errorCase(!existFile(FILE_TOOLKIT_EXE));
	}

	RemoveGitPaths(files);

	foreach (files, file, index)
	{
		if (!_stricmp(FLAG_FILE, getLocal(file)))
		{
			char *prefix = addChar(changeLocal(file, ""), '\\');
			char *subfile;
			uint subfile_index;

			foreach (files, subfile, subfile_index)
				if (startsWithICase(subfile, prefix))
					addElement(targets, (uint)subfile);

			memFree(prefix);

			distinct2(targets, simpleComp, noop_u);
		}
	}
	foreach (targets, file, index)
	{
		MaskResourceFile(file);
	}
	releaseDim(files, 1);
	releaseAutoList(targets);

	LOGPOS();
}
