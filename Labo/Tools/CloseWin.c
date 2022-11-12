/*
	CloseWin.exe [/-V] ...

		/-V ... ��\���̃E�B���h�E���Ώۂɂ���B(IsWindowVisible()�ɂ��`�F�b�N�����Ȃ�)

	CloseWin.exe ... EXE-NAME

		�E�B���h�E����܂��B

	CloseWin.exe ... /O  EXE-NAME
	CloseWin.exe ... /OT WIN-TITLE
	CloseWin.exe ... /C  EXE-NAME
	CloseWin.exe ... /CT WIN-TITLE

		/O, /OT ... �E�B���h�E���J�� (�v���Z�X���N������) �̂�҂��܂��B  �������ԂR��
		/C, /CT ... �E�B���h�E������ (�v���Z�X���I������) �̂�҂��܂��B�������ԂȂ�

		EXE-NAME  ... ���s�t�@�C���̃��[�J����, �^�X�N�}�l�[�W���̃C���[�W���Ƒ�������, �啶���E����������ʂ��Ȃ��B
		WIN-TITLE ... �E�B���h�E�̃^�C�g��, ������v, �啶���E����������ʂ��Ȃ��B

	CloseWin.exe ... /L

		�v���Z�X�̃��X�g��\�����邾���B

	CloseWin.exe ... /LT

		�E�B���h�E(�^�C�g��)�̃��X�g��\�����邾���B
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

		if (!mbs_stricmp(winTitle, P_WinTitle)) // ? �啶���������s��E���S��v�����B
//		if ( mbs_stristr(winTitle, P_WinTitle)) // ? �啶���������s��E������v�����B
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
	DWORD   dwSize;                // �\���̂̃T�C�Y
	DWORD   cntUsage;              // �v���Z�X�̎Q�ƃJ�E���g��
	DWORD   th32ProcessID;         // �v���Z�XID
	ULONG_PTR th32DefaultHeapID;   // �f�t�H���g�̃q�[�vID
	DWORD   th32ModuleID;          // ���W���[��ID
	DWORD   cntThreads;            // �X���b�h��
	DWORD   th32ParentProcessID;   // �e�v���Z�XID
	LONG    pcPriClassBase;        // ��{�D�揇�ʃ��x��
	DWORD   dwFlags;               // �t���O(���g�p:���0)
	CHAR    szExeFile[MAX_PATH];   // �t�@�C����(���s���W���[����)
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

			cout("���[�v�� [ %u ], F �������Ƌ����I�ɑ��s�����B(wait open)\n", loopCnt);

			if (waitKey(0) == 'F')
			{
				LOGPOS();
				break;
			}
			errorCase(90 <= loopCnt); // 90 * 2000 ms == �R��
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

			cout("���[�v�� [ %u ], F �������Ƌ����I�ɑ��s�����B(wait open title)\n", loopCnt);

			if (waitKey(0) == 'F')
			{
				LOGPOS();
				break;
			}
			errorCase(90 <= loopCnt); // 90 * 2000 ms == �R��
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
				cout("�O�ׂ̈�����x�m�F���܂��B(wait close)\n");

				coSleep(5000);

				FoundFlag = 0;
				SearchProcByExeName(exeName, FindPE);

				cout("FoundFlag.2: %d\n", FoundFlag);

				if (!FoundFlag)
					break;
			}
			cout("���[�v�� [ %u ], F �������Ƌ����I�ɑ��s�����B(wait close)\n", loopCnt);

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
				cout("�O�ׂ̈�����x�m�F���܂��B(wait close)\n");

				coSleep(5000);

				FoundFlag = 0;
				FindWinTitle(winTitle);

				cout("FoundFlag.2: %d\n", FoundFlag);

				if (!FoundFlag)
					break;
			}
			cout("���[�v�� [ %u ], F �������Ƌ����I�ɑ��s�����B(wait close title)\n", loopCnt);

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

			cout("���[�v�� [ %u ], F �������Ƌ����I�ɑ��s�����B\n", loopCnt);

			if (waitKey(0) == 'F')
			{
				LOGPOS();
				break;
			}
			errorCase(90 <= loopCnt); // 90 * 2000 ms == �R��
			coSleep(2000);
		}
	}
}
