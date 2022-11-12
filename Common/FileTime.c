#include "all.h"

static uint64 FileTimeToStamp(FILETIME *ft)
{
	FILETIME lft;
	SYSTEMTIME st;

	errorCase(!FileTimeToLocalFileTime(ft, &lft)); // ? é∏îs
	errorCase(!FileTimeToSystemTime(&lft, &st)); // ? é∏îs

	return
		st.wYear   * 10000000000000ui64 +
		st.wMonth  * 100000000000ui64 +
		st.wDay    * 1000000000ui64 +
		st.wHour   * 10000000ui64 +
		st.wMinute * 100000ui64 +
		st.wSecond * 1000ui64 +
		st.wMilliseconds;
}
static void StampToFileTime(uint64 stamp, FILETIME *out_ft)
{
	SYSTEMTIME st;
	FILETIME lft;

	st.wMilliseconds = stamp % 1000; stamp /= 1000;
	st.wSecond = stamp % 100; stamp /= 100;
	st.wMinute = stamp % 100; stamp /= 100;
	st.wHour   = stamp % 100; stamp /= 100;
	st.wDay    = stamp % 100; stamp /= 100;
	st.wMonth  = stamp % 100; stamp /= 100;
	st.wYear   = stamp;

	/*
		ïsê≥Ç»ì˙éûÇÕ SystemTimeToFileTime() Ç™ÉGÉâÅ[Ç…ÇµÇƒÇ≠ÇÍÇÈÅB
	*/
	errorCase(!SystemTimeToFileTime(&st, &lft)); // ? é∏îs
	errorCase(!LocalFileTimeToFileTime(&lft, out_ft)); // ? é∏îs
}
uint64 getFileStampByTime(time_t t)
{
	return toValue64_x(makeCompactStamp(getStampDataTime(t))) * 1000;
}
time_t getTimeByFileStamp(uint64 stamp)
{
	return compactStampToTime_x(xcout("%I64u", stamp / 1000));
}
void getFileStamp(char *file, uint64 *createTime, uint64 *accessTime, uint64 *updateTime)
{
	FILE *fp = fileOpen(file, "rb");
	HANDLE hdl;
	FILETIME ft[3];

	memset(ft, 0x00, sizeof(ft));

	hdl = getHandleByFilePointer(fp);
	errorCase(!GetFileTime(hdl, ft + 0, ft + 1, ft + 2)); // ? é∏îs

	fileClose(fp);

	if (createTime) *createTime = FileTimeToStamp(ft + 0);
	if (accessTime) *accessTime = FileTimeToStamp(ft + 1);
	if (updateTime) *updateTime = FileTimeToStamp(ft + 2);
}
void setFileStamp(char *file, uint64 createTime, uint64 accessTime, uint64 updateTime)
{
	FILE *fp = fileOpen(file, "r+b");
	FILETIME ft[3];
	FILETIME *pft[3];
	HANDLE hdl;

	memset(ft, 0x00, sizeof(ft));
	memset(pft, 0x00, sizeof(pft));

	if (createTime) StampToFileTime(createTime, pft[0] = ft + 0);
	if (accessTime) StampToFileTime(accessTime, pft[1] = ft + 1);
	if (updateTime) StampToFileTime(updateTime, pft[2] = ft + 2);

	hdl = getHandleByFilePointer(fp);
	errorCase(!SetFileTime(hdl, pft[0], pft[1], pft[2])); // ? é∏îs

	fileClose(fp);
}
