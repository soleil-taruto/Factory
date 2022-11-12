/*
	CloseWin.exe [/-V] ...

		/-V ... 非表示のウィンドウも対象にする。(IsWindowVisible()によるチェックをしない)

	CloseWin.exe ... EXE-NAME

		ウィンドウを閉じます。

	CloseWin.exe ... /O  EXE-NAME
	CloseWin.exe ... /OT WIN-TITLE
	CloseWin.exe ... /C  EXE-NAME
	CloseWin.exe ... /CT WIN-TITLE

		/O, /OT ... ウィンドウが開く (プロセスが起動する) のを待ちます。  制限時間３分
		/C, /CT ... ウィンドウが閉じる (プロセスが終了する) のを待ちます。制限時間なし

		EXE-NAME  ... 実行ファイルのローカル名, タスクマネージャのイメージ名と多分同じ, 大文字・小文字を区別しない。
		WIN-TITLE ... ウィンドウのタイトル, 部分一致, 大文字・小文字を区別しない。

	CloseWin.exe ... /L

		プロセスのリストを表示するだけ。

	CloseWin.exe ... /LT

		ウィンドウ(タイトル)のリストを表示するだけ。
*/

#pragma comment(lib, "user32.lib")

#include "C:\Factory\Common\all.h"
#include <tlhelp32.h>

#define PROCPATH_LENMAX 2000
#define WINTITLE_LENMAX 1000

static int NoCheckVisible;
static int WinClosedFlag;
static int FoundFlag;

static PROCESSENTRY32 P_Pe32;
static char *P_WinTitle;

static BOOL CALLBACK EnumDoCloseWin(HWND hWnd, LPARAM lp)
{
	cout("hWnd: %u\n", hWnd);

	if (NoCheckVisible || IsWindowVisible(hWnd))
	{
		DWORD procId;

		GetWindowThreadProcessId(hWnd, &procId);

		cout("procId: %u (%u)\n", procId, P_Pe32.th32ProcessID);

		if (procId == P_Pe32.th32ProcessID)
		{
			LOGPOS();
			SendMessage(hWnd, WM_CLOSE, (WPARAM)NULL, (LPARAM)NULL);
			WinClosedFlag = 1;
		}
	}
	return TRUE;
}
static void DoCloseWin(PROCESSENTRY32 pe32)
{
	P_Pe32 = pe32;
	EnumWindows(EnumDoCloseWin, (LPARAM)NULL);
}
static BOOL CALLBACK EnumFindWinTitle(HWND hWnd, LPARAM lp)
{
	cout("hWnd: %u\n", hWnd);

	if (NoCheckVisible || IsWindowVisible(hWnd))
	{
		char winTitle[WINTITLE_LENMAX + 1];

		GetWindowText(hWnd, winTitle, WINTITLE_LENMAX);
		line2JLine(winTitle, 1, 0, 0, 1);

		cout("winTitle: [%s]\n", winTitle);

		if (!mbs_stricmp(winTitle, P_WinTitle)) // ? 大文字小文字不問・完全一致した。
//		if ( mbs_stristr(winTitle, P_WinTitle)) // ? 大文字小文字不問・部分一致した。
		{
			LOGPOS();
			FoundFlag = 1;
		}
	}
	return TRUE;
}
static BOOL CALLBACK PrintWinTitle(HWND hWnd, LPARAM lp)
{
	char winTitle[WINTITLE_LENMAX + 1];

	GetWindowText(hWnd, winTitle, WINTITLE_LENMAX);
	line2JLine(winTitle, 1, 0, 0, 1);

	cout("----\n");
	cout("hWnd: %u\n", hWnd);
	cout("IsWindowVisible: %d\n", IsWindowVisible(hWnd) ? 1 : 0);
	cout("winTitle: [%s]\n", winTitle);
	cout("----\n");

	return TRUE;
}
static void FindWinTitle(char *winTitle)
{
	P_WinTitle = winTitle;
	EnumWindows(EnumFindWinTitle, (LPARAM)NULL);
}
static void FindPE(PROCESSENTRY32 pe32)
{
	FoundFlag = 1;
}

static void SearchProcByExeName(char *exeName, void (*perform)(PROCESSENTRY32 pe32))
{
	HANDLE hSnapshot;

	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hSnapshot != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 pe32;

		pe32.dwSize = sizeof(PROCESSENTRY32);

		if (Process32First(hSnapshot, &pe32))
		{
			do
			{
/*
typedef struct tagPROCESSENTRY32 {
	DWORD   dwSize;                // 構造体のサイズ
	DWORD   cntUsage;              // プロセスの参照カウント数
	DWORD   th32ProcessID;         // プロセスID
	ULONG_PTR th32DefaultHeapID;   // デフォルトのヒープID
	DWORD   th32ModuleID;          // モジュールID
	DWORD   cntThreads;            // スレッド数
	DWORD   th32ParentProcessID;   // 親プロセスID
	LONG    pcPriClassBase;        // 基本優先順位レベル
	DWORD   dwFlags;               // フラグ(未使用:常に0)
	CHAR    szExeFile[MAX_PATH];   // ファイル名(実行モジュール名)
} PROCESSENTRY32, *LPPROCESSENTRY32;
*/
				cout("----\n");
				cout("szExeFile: %s\n", pe32.szExeFile);
				cout("cntUsage: %u\n", pe32.cntUsage);
				cout("cntThreads: %u\n", pe32.cntThreads);
				cout("th32ParentProcessID: %u\n", pe32.th32ParentProcessID);
				cout("th32ProcessID: %u\n", pe32.th32ProcessID);
				cout("th32DefaultHeapID: %u\n", pe32.th32DefaultHeapID);
				cout("th32ModuleID: %u\n", pe32.th32ModuleID);
				cout("pcPriClassBase: %u\n", pe32.pcPriClassBase);
				cout("dwFlags: %08x\n", pe32.dwFlags);
				cout("----\n");

				if (!_stricmp(pe32.szExeFile, exeName))
				{
					LOGPOS();
					perform(pe32);
				}
			}
			while (Process32Next(hSnapshot, &pe32));
		}
		CloseHandle(hSnapshot);
	}
}

int main(int argc, char **argv)
{
	char *exeName;
	uint loopCnt;
	int waitOpenMode = 0;
	int waitOpenTitleMode = 0;
	int waitCloseMode = 0;
	int waitCloseTitleMode = 0;
	char *winTitle;

	if (argIs("/-V"))
	{
		NoCheckVisible = 1;
	}
	if (argIs("/L"))
	{
		SearchProcByExeName("", (void (*)(PROCESSENTRY32 pe32))noop);
		return;
	}
	if (argIs("/LT"))
	{
		EnumWindows(PrintWinTitle, (LPARAM)NULL);
		return;
	}
	if (argIs("/O"))
	{
		waitOpenMode = 1;
		exeName = nextArg();
	}
	if (argIs("/OT"))
	{
		waitOpenTitleMode = 1;
		winTitle = nextArg();
	}
	if (argIs("/C"))
	{
		waitCloseMode = 1;
		exeName = nextArg();
	}
	if (argIs("/CT"))
	{
		waitCloseTitleMode = 1;
		winTitle = nextArg();
	}

	if (waitOpenMode)
	{
		for (loopCnt = 1; ; loopCnt++)
		{
			FoundFlag = 0;
			SearchProcByExeName(exeName, FindPE);

			cout("FoundFlag: %d\n", FoundFlag);

			if (FoundFlag)
				break;

			cout("ループ回数 [ %u ], F を押すと強制的に続行するよ。(wait open)\n", loopCnt);

			if (waitKey(0) == 'F')
			{
				LOGPOS();
				break;
			}
			errorCase(90 <= loopCnt); // 90 * 2000 ms == ３分
			coSleep(2000);
		}
		LOGPOS();
		return;
	}

	if (waitOpenTitleMode)
	{
		for (loopCnt = 1; ; loopCnt++)
		{
			FoundFlag = 0;
			FindWinTitle(winTitle);

			cout("FoundFlag: %d\n", FoundFlag);

			if (FoundFlag)
				break;

			cout("ループ回数 [ %u ], F を押すと強制的に続行するよ。(wait open title)\n", loopCnt);

			if (waitKey(0) == 'F')
			{
				LOGPOS();
				break;
			}
			errorCase(90 <= loopCnt); // 90 * 2000 ms == ３分
			coSleep(2000);
		}
		LOGPOS();
		return;
	}

	if (waitCloseMode)
	{
		for (loopCnt = 1; ; loopCnt++)
		{
			FoundFlag = 0;
			SearchProcByExeName(exeName, FindPE);

			cout("FoundFlag: %d\n", FoundFlag);

			if (!FoundFlag)
			{
				cout("念の為もう一度確認します。(wait close)\n");

				coSleep(5000);

				FoundFlag = 0;
				SearchProcByExeName(exeName, FindPE);

				cout("FoundFlag.2: %d\n", FoundFlag);

				if (!FoundFlag)
					break;
			}
			cout("ループ回数 [ %u ], F を押すと強制的に続行するよ。(wait close)\n", loopCnt);

			if (coWaitKey(25000) == 'F')
			{
				LOGPOS();
				break;
			}
		}
		LOGPOS();
		return;
	}

	if (waitCloseTitleMode)
	{
		for (loopCnt = 1; ; loopCnt++)
		{
			FoundFlag = 0;
			FindWinTitle(winTitle);

			cout("FoundFlag: %d\n", FoundFlag);

			if (!FoundFlag)
			{
				cout("念の為もう一度確認します。(wait close)\n");

				coSleep(5000);

				FoundFlag = 0;
				FindWinTitle(winTitle);

				cout("FoundFlag.2: %d\n", FoundFlag);

				if (!FoundFlag)
					break;
			}
			cout("ループ回数 [ %u ], F を押すと強制的に続行するよ。(wait close title)\n", loopCnt);

			if (waitKey(25000) == 'F')
			{
				LOGPOS();
				break;
			}
		}
		LOGPOS();
		return;
	}

	exeName = nextArg();

	{
		for (loopCnt = 1; ; loopCnt++)
		{
			WinClosedFlag = 0;
			SearchProcByExeName(exeName, DoCloseWin);

			cout("WinClosedFlag: %d\n", WinClosedFlag);

			if (!WinClosedFlag)
				break;

			cout("ループ回数 [ %u ], F を押すと強制的に続行するよ。\n", loopCnt);

			if (waitKey(0) == 'F')
			{
				LOGPOS();
				break;
			}
			errorCase(90 <= loopCnt); // 90 * 2000 ms == ３分
			coSleep(2000);
		}
	}
}
