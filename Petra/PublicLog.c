/*
	PublicLog.exe �R�����g...
*/

#include "C:\Factory\Common\all.h"

#define LOG_FILE    "C:\\vaz\\PublicLog.txt"
#define LOG_FILE_02 "C:\\vaz\\PublicLog2.txt"
#define LOG_FILE_03 "C:\\vaz\\PublicLog3.txt"
/*
#define LOG_FILE    "C:\\tmp\\PublicLog.txt"
#define LOG_FILE_02 "C:\\tmp\\PublicLog2.txt"
#define LOG_FILE_03 "C:\\tmp\\PublicLog3.txt"
*/

#define LOG_FILE_SIZE_MAX 8000000

int main(int argc, char **argv)
{
	char *message = untokenize(allArgs(), " "); // g -- memo: allArgs() �� argv �̐󂢃R�s�[�Ȃ̂� untokenize_xc �� ng

	mutex();
	{
		if(existFile(LOG_FILE) && LOG_FILE_SIZE_MAX < getFileSize(LOG_FILE))
		{
			if(existFile(LOG_FILE_02))
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
