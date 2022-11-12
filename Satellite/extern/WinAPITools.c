#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\PadFile.h"
#include "C:\Factory\Satellite\libs\Flowertact\Fortewave.h"
#include <tlhelp32.h>

#define MTX_EXTRACT "{aed96b6d-8a77-40fb-9285-9b75405fc3b2}"

// ---- PadFile ----

#define PAD_LABEL "{356e9638-f79b-4a7b-a9b6-57916b8118a9}"

static void WAT_PadFile(char *file)
{
	PadFile2(file, PAD_LABEL);
}
static void WAT_UnpadFile(char *file)
{
	if (existFile(file))
	{
		if (!UnpadFile2(file, PAD_LABEL))
		{
			removeFile(file);
		}
	}
}

// ---- Process ----

static char *ParentProcMonitorName;

static int IsParentAlive(void)
{
	int alive = 1;
	uint hdl = mutexOpen(ParentProcMonitorName);

	if (handleWaitForMillis(hdl, 0)) // ? ロックできた。== ロックされていない。-> 親プロセス停止中
	{
		alive = 0;
		mutexRelease(hdl);
	}
	handleClose(hdl);
	return alive;
}
static int IsProcessAlive(uint targetProcId)
{
	int alive = 0;
	HANDLE hSnapshot;
	PROCESSENTRY32 pe32;

	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hSnapshot != INVALID_HANDLE_VALUE)
	{
		pe32.dwSize = sizeof(PROCESSENTRY32);

		if (Process32First(hSnapshot, &pe32))
		{
			do
			{
				int procId = (int)pe32.th32ProcessID;

				if (procId == targetProcId)
					alive = 1;
			}
			while (Process32Next(hSnapshot, &pe32));
		}
		CloseHandle(hSnapshot);
	}
	return alive;
}

// ----

static int HdlWaitForMillisEx(int hdl, uint millis)
{
	uint elapse = 0;

	for (; ; )
	{
		if (millis == INFINITE)
		{
			if (handleWaitForMillis(hdl, 2000))
				return 1;
		}
		else
		{
			uint m = m_min(2000, millis - elapse);

			if (handleWaitForMillis(hdl, m))
				return 1;

			elapse += m;

			if (millis <= elapse)
				break;
		}
		if (!IsParentAlive())
			break;
	}
	return 0;
}
int main(int argc, char **argv)
{
	if (argIs("/MUTEX-WAIT-ONE"))
	{
		char *targetName;
		uint millis;
		char *beganName;
		char *wObj0File;
		char *endName;
		uint targetMutex;
		uint beganEvent;
		uint endEvent;

		targetName = nextArg();
		millis = toValue(nextArg());
		beganName = nextArg();
		wObj0File = nextArg();
		endName = nextArg();
		ParentProcMonitorName = nextArg();

		targetMutex = mutexOpen(targetName);
		beganEvent = eventOpen(beganName);
		endEvent = eventOpen(endName);

		if (HdlWaitForMillisEx(targetMutex, millis))
		{
			createFile(wObj0File);
			eventSet(beganEvent);
			HdlWaitForMillisEx(endEvent, INFINITE);
			mutexRelease(targetMutex);
		}
		else
		{
			eventSet(beganEvent);
			HdlWaitForMillisEx(endEvent, INFINITE);
		}
		handleClose(targetMutex);
		handleClose(beganEvent);
		handleClose(endEvent);
		return;
	}
	if (argIs("/EVENT-CREATE"))
	{
		char *targetName;
		char *beganName;
		char *endName;
		uint targetEvent;
		uint beganEvent;
		uint endEvent;

		targetName = nextArg();
		beganName = nextArg();
		endName = nextArg();
		ParentProcMonitorName = nextArg();

		targetEvent = eventOpen(targetName);
		beganEvent = eventOpen(beganName);
		endEvent = eventOpen(endName);

		eventSet(beganEvent);
		HdlWaitForMillisEx(endEvent, INFINITE);

		handleClose(targetEvent);
		handleClose(beganEvent);
		handleClose(endEvent);
		return;
	}
	if (argIs("/EVENT-SET"))
	{
		char *targetName;
		uint targetEvent;

		targetName = nextArg();

		targetEvent = eventOpen(targetName);

		eventSet(targetEvent);

		handleClose(targetEvent);
		return;
	}
	if (argIs("/EVENT-WAIT-ONE"))
	{
		char *targetName;
		uint millis;
		uint targetEvent;

		targetName = nextArg();
		millis = toValue(nextArg());
		ParentProcMonitorName = nextArg();

		targetEvent = eventOpen(targetName);

		HdlWaitForMillisEx(targetEvent, millis);

		handleClose(targetEvent);
		return;
	}
	if (argIs("/GET-ENV"))
	{
		char *envName;
		char *envValFile;
		char *envVal;

		envName = nextArg();
		envValFile = nextArg();

		envVal = getEnvLine(envName);

		writeOneLineNoRet(envValFile, envVal);
		return;
	}
	if (argIs("/DEAD-AND-REMOVE"))
	{
		char *beganName;
		char *deadName;
		char *mtxName;
		char *targetPath;
		uint beganEvent;
		uint deadEvent;
		uint mtx;

		beganName = nextArg();
		deadName = nextArg();
		mtxName = nextArg();
		targetPath = nextArg();
		ParentProcMonitorName = nextArg();

		beganEvent = eventOpen(beganName);
		deadEvent = eventOpen(deadName);
		mtx = mutexOpen(mtxName);

		eventSet(beganEvent);
		HdlWaitForMillisEx(deadEvent, INFINITE);

		handleWaitForever(mtx);
		recurRemovePathIfExist(targetPath);
		mutexRelease(mtx);

		handleClose(beganEvent);
		handleClose(deadEvent);
		handleClose(mtx);
		return;
	}
	if (argIs("/DELETE-DELAY-UNTIL-REBOOT"))
	{
		char *targetPath;
		char *delayFile;
		uint hdl;

		targetPath = nextArg();
		delayFile = nextArg();

		hdl = mutexLock(MTX_EXTRACT);
		{
			WAT_UnpadFile(delayFile);
			addLine2File(delayFile, targetPath);
			WAT_PadFile(delayFile);
		}
		mutexUnlock(hdl);
//		MoveFileEx(targetPath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT); // old
		return;
	}
	if (argIs("/CHECK-PROCESS-ALIVE"))
	{
		uint targetProcId;
		char *trueFile;

		targetProcId = toValue(nextArg());
		trueFile = nextArg();

		if (IsProcessAlive(targetProcId))
			createFile(trueFile);

		return;
	}
	if (argIs("/CHECK-MUTEX-LOCKED"))
	{
		char *targetName;
		char *trueFile;
		uint mtx;

		targetName = nextArg();
		trueFile = nextArg();

		mtx = mutexOpen(targetName);

		if (handleWaitForMillis(mtx, 0))
			mutexRelease(mtx);
		else
			createFile(trueFile);

		handleClose(mtx);
		return;
	}
	if (argIs("/SEND-TO-FORTEWAVE"))
	{
		char *identHash;
		char *dir;

		identHash = nextArg();
		dir = nextArg();

		{
			Frtwv_t *i = Frtwv_CreateIH(identHash);
			uint index;

			for (index = 0; ; index++)
			{
				char *file = combine_cx(dir, xcout("%04u", index));
				autoBlock_t *sendData;

				if (!existFile(file))
				{
					memFree(file);
					break;
				}
				sendData = readBinary(file);
				Frtwv_Send(i, sendData);
				releaseAutoBlock(sendData);
				removeFile(file);
				memFree(file);
			}
			Frtwv_Release(i);
		}
		removeDir(dir);
		return;
	}
	if (argIs("/RECV-FROM-FORTEWAVE"))
	{
		char *identHash;
		char *dir;
		uint millis;
		uint recvLimit;
		uint recvLimitSize;

		identHash = nextArg();
		dir = nextArg();
		millis = toValue(nextArg());
		recvLimit = toValue(nextArg());
		recvLimitSize = toValue(nextArg());

		{
			Frtwv_t *i = Frtwv_CreateIH(identHash);
			uint index;
			uint totalSize = 0;

			for (index = 0; index < recvLimit && totalSize < recvLimitSize; index++)
			{
				autoBlock_t *recvData = Frtwv_Recv(i, index ? 0 : millis);
				char *file;

				if (!recvData)
					break;

				file = combine_cx(dir, xcout("%04u", index));
				writeBinary(file, recvData);
				totalSize += getSize(recvData);
				releaseAutoBlock(recvData);
				memFree(file);
			}
			Frtwv_Release(i);
		}
		return;
	}
	if (argIs("/EXTRACT"))
	{
		char *rFile;
		char *wFile;
		char *delayFile;
		char *lastExtractedTimeFile;
		uint hdl;

		rFile = nextArg();
		wFile = nextArg();
		delayFile = nextArg();
		lastExtractedTimeFile = xcout("%s.last-extracted-time", delayFile);

		hdl = mutexLock(MTX_EXTRACT);
		{
			time_t currTime = time(NULL);
			time_t bootTime;

			bootTime = currTime - now();

			// ? 最後に展開したときから2日以上経過してからPCを起動して最初にここへ到達した。-> 展開する。
			// 時刻調整や誤差を考慮してマージンとしての2日
			if (
				!existFile(lastExtractedTimeFile) ||
				getFileSize(lastExtractedTimeFile) != 8 ||
				readFirstValue64(lastExtractedTimeFile) + 86400 * 2 < bootTime

|| _access(wFile, 0) // そもそも存在しなければ展開する。@ 2018.11.21

				)
			{
				createPath(wFile, 'X');
				removeFileIfExist(wFile);
				moveFile(rFile, wFile);

				WAT_UnpadFile(delayFile);

				if (existFile(delayFile)) // 次の再起動時まで待たされた削除を実行する。
				{
					FILE *fp = fileOpen(delayFile, "rt");

					for (; ; )
					{
						char *targetPath = readLine(fp);

						if (!targetPath)
							break;

						remove(targetPath);
						execute_x(xcout("RD /S /Q \"%s\"", targetPath));
						memFree(targetPath);
					}
					fileClose(fp);
					removeFile(delayFile);
				}
				writeOneValue64(lastExtractedTimeFile, currTime); // 今回の展開時刻を保存する。
			}
		}
		mutexUnlock(hdl);

		memFree(lastExtractedTimeFile);
		return;
	}
	if (argIs("/MONITOR"))
	{
		uint parentProcId;
		uint hdl;
		uint mtx;

		parentProcId = toValue(nextArg());
		ParentProcMonitorName = nextArg();

		hdl = (uint)OpenProcess(PROCESS_ALL_ACCESS, 0, parentProcId);
		errorCase(hdl == 0);

		mtx = mutexOpen(ParentProcMonitorName);

		handleWaitForever(mtx);
		handleWaitForever(hdl);
		mutexRelease(mtx);

		handleClose(hdl);
		handleClose(mtx);
		return;
	}
	cout("Dummy cout: {cfb94d47-7371-4080-a0b2-c3c4c6deafd6}\n"); // shared_uuid@g
}
