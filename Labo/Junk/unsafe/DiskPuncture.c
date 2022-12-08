#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\CryptoRand.h"

/*
	値域: 0 ～
	試した限りではシステムドライブが50MBを切ると激しく警告してくる。
*/
static uint64 BorderSize = 50 * 1024 * 1024;

static int TargetDrive;
static char *TargetRootDir;
static char *GarbageFilesDir;
static autoList_t *GarbageFiles;
static char *OutputDir;

static int CheckBorderSize(uint64 nextSize)
{
	updateDiskSpace(TargetDrive);

	cout("空き領域 %I64u (下限 %I64u + %I64u = %I64u)\n", lastDiskFree, BorderSize, nextSize, BorderSize + nextSize);

	return BorderSize + nextSize < lastDiskFree;
}
static char *GetOutputFile(char *dir)
{
	char *lclfile = strx("");
	uint count;

	for (count = 0; count < 16; count++)
	{
		lclfile = addLine_x(lclfile, xcout("%02x", getCryptoByte()));
	}
	lclfile = addExt(lclfile, "pad");
	return toCreatablePath(combine_cx(dir, lclfile), 10);
}
static void CopyGarbageFiles(void)
{
	char *outDir = toCreatablePath(xcout("%c:\\1", TargetDrive), 10000);
	char *file;
	uint index;

	createDir(outDir);

	foreach (GarbageFiles, file, index)
	{
		while (CheckBorderSize(getFileSize(file)))
		{
			char *outFile = GetOutputFile(outDir);
			uint startTime = now();

			cout("< %s\n", file);
			cout("> %s\n", outFile);

			execute_x(xcout("COPY %s %s", file, outFile));

			cout("%u SEC\n", now() - startTime);

			if (!existFile(outFile))
			{
				memFree(outFile);
				break;
			}
			memFree(outFile);
		}
	}
	OutputDir = outDir;
}

static void MakeGarbageFiles(void)
{
	uint index;

	GarbageFilesDir = makeTempPath(NULL);
	GarbageFiles = newList();

	createDir(GarbageFilesDir);

	for (index = 0; index < 20; index++)
	{
		char *file = combine_cx(GarbageFilesDir, xcout("%02u.pad", index));

		cout("> %s\n", file);

		if (index)
		{
			char *halfFile = getLine(GarbageFiles, getCount(GarbageFiles) - 1);

			execute_x(xcout("COPY /B %s + %s %s", halfFile, halfFile, file));

			errorCase(getFileSize(file) != getFileSize(halfFile) * 2);
		}
		else
		{
			FILE *fp = fileOpen(file, "wb");
			uint count;

			for (count = 4096; count; count--)
			{
				writeChar(fp, 0xaa);
			}
			fileClose(fp);
		}
		addElement(GarbageFiles, (uint)file);
	}
	reverseElements(GarbageFiles); // 2GB -> 4KB
}
static DeleteGarbageFiles(void)
{
	char *file;
	uint index;
	uint sameDrv = c2upper(getLine(GarbageFiles, 0)[0]) == c2upper(TargetDrive);

	foreach (GarbageFiles, file, index)
	{
		if (sameDrv)
		{
			char *outFile = GetOutputFile(OutputDir);

			cout("< %s\n", file);
			cout("> %s\n", outFile);

			execute_x(xcout("MOVE %s %s", file, outFile));

			memFree(outFile);
		}
		else
		{
			removeFile(file);
		}
	}
	removeDir(GarbageFilesDir);
}

static void DiskPuncture(int drive)
{
	uint size;
	char *file;
	uint index;

	errorCase(!m_isalpha(drive));
	TargetDrive = drive;
	TargetRootDir = xcout("%c:\\", drive);
	errorCase(!existDir(TargetRootDir));

	cout("BORDER SIZE %I64u\n", BorderSize);
	cout("DISK PUNCTURE %s CONTINUE?\n", TargetRootDir);

	if (clearGetKey() == 0x1b)
		termination(0);

	MakeGarbageFiles();
	CopyGarbageFiles();
	DeleteGarbageFiles();

	cout("END\n");

	memFree(TargetRootDir);
	memFree(GarbageFilesDir);
	releaseDim(GarbageFiles, 1);
	memFree(OutputDir);
}

int main(int argc, char **argv)
{
	if (argIs("/S"))
	{
		BorderSize = toValue64(nextArg());
	}

	if (hasArgs(1))
	{
		if (getArg(0)[0] == '.')
		{
			DiskPuncture(c_getCwd()[0]);
		}
		else
		{
			DiskPuncture(nextArg()[0]);
		}
	}
	else
	{
		DiskPuncture(c_dropDirFile()[0]);
	}
}
