/*
	PublicLog.exe コメント...
*/

#include "C:\Factory\Common\all.h"

#define LOG_FILE    "C:\\Factory\\tmp\\PublicLog.txt"
#define LOG_FILE_02 "C:\\Factory\\tmp\\PublicLog2.txt"
#define LOG_FILE_03 "C:\\Factory\\tmp\\PublicLog3.txt"

// del @ 2022.1.23
/*
#define LOG_FILE    "C:\\temp\\PublicLog.txt"
#define LOG_FILE_02 "C:\\temp\\PublicLog2.txt"
#define LOG_FILE_03 "C:\\temp\\PublicLog3.txt"
*/

#define LOG_FILE_SIZE_MAX 8000000

int main(int argc, char **argv)
{
	char *message = untokenize(allArgs(), " "); // g -- memo: allArgs() は argv の浅いコピーなので untokenize_xc は ng

	mutex();
	{
		if (existFile(LOG_FILE) && LOG_FILE_SIZE_MAX < getFileSize(LOG_FILE))
		{
			if (existFile(LOG_FILE_02))
			{
				removeFileIfExist(
					LOG_FILE_03
					);

				moveFile(
					LOG_FILE_02,
					LOG_FILE_03
					);
			}
			moveFile(
				LOG_FILE,
				LOG_FILE_02
				);
		}
		addLine2File_cx(LOG_FILE, xcout("[%s] %s", c_makeJStamp(NULL, 0), message));
	}
	unmutex();

	memFree(message);
}
