#include "C:\Factory\Common\all.h"

#define CLUSTER_EXE "C:\\Factory\\Tools\\Cluster.exe"

static uint64 GetI64Stamp(time_t t)
{
	return toValue64_x(makeCompactStamp(getStampDataTime(t)));
}
static void MakeReportFile(char *dir, char *repFile)
{
	autoList_t *report = newList();
	autoList_t *paths;
	char *path;
	uint index;

	// 2bs
	errorCase(lsDirAction);
	errorCase(lsFileAction);
	errorCase(lsInfos);

	dir = makeFullPath(dir);
	paths = lss(dir);
	sortJLinesICase(paths);

	foreach (paths, path, index)
	{
		char *line;

		updateFindData(path);

		line = xcout(
			"%c%c%c%c%c "
			,lastFindData.attrib & _A_ARCH   ? 'A' : ' '
			,lastFindData.attrib & _A_HIDDEN ? 'H' : ' '
			,lastFindData.attrib & _A_RDONLY ? 'R' : ' '
			,lastFindData.attrib & _A_SUBDIR ? 'D' : ' '
			,lastFindData.attrib & _A_SYSTEM ? 'S' : ' '
			);

		if (existFile(path))
			line = addLine_x(line, xcout(
				"%I64d %I64d %I64d "
				,GetI64Stamp(lastFindData.time_create)
				,GetI64Stamp(lastFindData.time_access)
				,GetI64Stamp(lastFindData.time_write)
				));

		line = addLine(line, eraseRoot(path, dir));

		addElement(report, (uint)line);
	}
	writeLines(repFile, report);

	memFree(dir);
	releaseDim(paths, 1);
	releaseDim(report, 1);
}
static void MakeClusterI(char *rDir, char *wFile)
{
	coExecute_x(xcout("%s /I /M \"%s\" \"%s\"", CLUSTER_EXE, wFile, rDir));
}
static void UnmakeClusterI(char *rFile, char *wDir)
{
	coExecute_x(xcout("%s /R \"%s\" \"%s\"", CLUSTER_EXE, rFile, wDir));
}
static void ClusterWithInfoTest(char *dir)
{
	char *repFileBase = makeTempPath(NULL);
	char *repFile1;
	char *repFile2;
	char *mFile = makeTempPath(NULL);
	char *wDir = makeTempPath(NULL);

	repFile1 = xcout("%s.ClusterWithInfo-Test-Report-1.txt", repFileBase);
	repFile2 = xcout("%s.ClusterWithInfo-Test-Report-2.txt", repFileBase);

	MakeReportFile(dir, repFile1);

	MakeClusterI(dir, mFile);
	UnmakeClusterI(mFile, wDir);

	MakeReportFile(wDir, repFile2);

	errorCase(!isSameFile(repFile1, repFile2));

	// ëSïîçÌèúÇµÇ»Ç¢ÅB

	memFree(repFileBase);
	memFree(repFile1);
	memFree(repFile2);
	memFree(mFile);
	memFree(wDir);
}
int main(int argc, char **argv)
{
	ClusterWithInfoTest(c_dropDir());
}
