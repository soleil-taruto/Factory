/*
	DyIPChk.exe チェックするドメイン名
*/

#include "C:\Factory\Common\all.h"

#define SAVE_DATA_DIR "C:\\Factory\\tmp\\DyIPChk_SaveData"
//#define SAVE_DATA_DIR "C:\\appdata\\DyIPChk_SaveData" // appdata 抑止 @ 2021.3.19
#define COMMON_SAVE_DATA_FILE "C:\\Factory\\tmp\\DyIPChk_SaveData.txt"
//#define COMMON_SAVE_DATA_FILE "C:\\appdata\\DyIPChk_SaveData.txt" // appdata 抑止 @ 2021.3.19

#define DY_IP_CHK_COMMAND "C:\\Factory\\Labo\\Socket\\hget.exe /L http://ieserver.net/ipcheck.shtml IP.tmp"
#define IP_OUT_FILE "IP.tmp"

static uint NoIPCount;
static time_t LastUpdatedTime;
static char *RecentDyIP;
static time_t RecentDyIPGotTime;

static char *GetSaveDataFile(char *domain)
{
	return combine(SAVE_DATA_DIR, domain);
}
static void LoadData(char *domain)
{
	char *saveDataFile = GetSaveDataFile(domain);

	if (existFile(saveDataFile))
	{
		FILE *fp = fileOpen(saveDataFile, "rt");

		NoIPCount = toValue_x(neReadLine(fp));
		LastUpdatedTime = toValue64_x(neReadLine(fp));

		fileClose(fp);
	}
	else
	{
		NoIPCount = 0;
		LastUpdatedTime = 0;
	}
	memFree(saveDataFile);

	if (existFile(COMMON_SAVE_DATA_FILE))
	{
		FILE *fp = fileOpen(COMMON_SAVE_DATA_FILE, "rt");

		RecentDyIP = neReadLine(fp);
		RecentDyIPGotTime = toValue64_x(neReadLine(fp));

		fileClose(fp);
	}
	else
	{
		RecentDyIP = strx("x.x.x.x");
		RecentDyIPGotTime = 0;
	}
}
static void SaveData(char *domain)
{
	char *saveDataFile = GetSaveDataFile(domain);

	createDirIfNotExist(SAVE_DATA_DIR);

	{
		FILE *fp = fileOpen(saveDataFile, "wt");

		writeLine_x(fp, xcout("%u", NoIPCount));
		writeLine_x(fp, xcout("%I64d", LastUpdatedTime));

		fileClose(fp);
	}

	memFree(saveDataFile);

	{
		FILE *fp = fileOpen(COMMON_SAVE_DATA_FILE, "wt");

		writeLine_x(fp, xcout("%s", RecentDyIP));
		writeLine_x(fp, xcout("%I64d", RecentDyIPGotTime));

		fileClose(fp);
	}
}
static char *GetIP_x(char *command)
{
	char *dir = makeTempDir("DyIPChk.tmp");
	char *ip = NULL;

	addCwd(dir);
	{
		coExecute(command);

		if (existFile(IP_OUT_FILE))
		{
			ip = readFirstLine(IP_OUT_FILE);

			coutJLine_x(xcout("Recved_IP.1=[%s]", ip)); // test

			if (!lineExp("<1,3,09>.<1,3,09>.<1,3,09>.<1,3,09>", ip)) // ? ! IP address
			{
				memFree(ip);
				ip = NULL;
			}
			removeFile(IP_OUT_FILE);
		}
	}
	unaddCwd();

	removeDir(dir);

	memFree(dir);
	memFree(command);

	coutJLine_x(xcout("Recved_IP.2=[%s]", ip ? ip : "<NULL>")); // test

	return ip;
}
static char *GetDyIP(void)
{
	return GetIP_x(strx(DY_IP_CHK_COMMAND));
}
static char *GetDomainIP(char *domain)
{
	return GetIP_x(xcout("C:\\Factory\\Tools\\Lookup.exe %s > %s", domain, IP_OUT_FILE));
}
static void CheckDyIP(char *domain)
{
	uint retCode = 0;
	char *dyIP;
	char *domainIP;
	time_t currTime = time(NULL);

	cout("domain: %s\n", domain);

	LoadData(domain);

	cout("L.NoIPCount: %u\n", NoIPCount);
	cout("L.LastUpdatedTime: %I64d\n", LastUpdatedTime);
	cout("L.RecentDyIP: %s\n", RecentDyIP);
	cout("L.RecentDyIPGotTime: %I64d\n", RecentDyIPGotTime);

	cout("currTime - RecentDyIPGotTime == %I64d\n", currTime - RecentDyIPGotTime);

	if (RecentDyIPGotTime + 90 < currTime) // ? timeout
	{
		dyIP = GetDyIP();

		if (dyIP)
		{
			memFree(RecentDyIP);
			RecentDyIP = strx(dyIP);
			RecentDyIPGotTime = currTime;
		}
	}
	else
	{
		dyIP = strx(RecentDyIP);

		coutJLine_x(xcout("Recved_IP.C=[%s]", dyIP)); // test // HACK: cout で良いんじゃない？
	}
	domainIP = GetDomainIP(domain);

	if (dyIP && domainIP)
	{
		if (strcmp(dyIP, domainIP))
		{
			cout("IP不一致\n");
			retCode = 1;
		}
		NoIPCount = 0;
	}
	else
	{
		NoIPCount++;
	}

	if (3 <= NoIPCount)
//	if (10 < NoIPCount)
		retCode = 1;

	cout("currTime - LastUpdatedTime == %.3f\n", (currTime - LastUpdatedTime) / 86400.0);

	if (LastUpdatedTime + 35 * 86400 < currTime) // ? timeout
		retCode = 1;

	if (retCode)
	{
		NoIPCount = 0;
		LastUpdatedTime = currTime;
	}
	cout("S.NoIPCount: %u\n", NoIPCount);
	cout("S.LastUpdatedTime: %I64d\n", LastUpdatedTime);
	cout("S.RecentDyIP: %s\n", RecentDyIP);
	cout("S.RecentDyIPGotTime: %I64d\n", RecentDyIPGotTime);

	SaveData(domain);

	memFree(dyIP);
	memFree(domainIP);
	cout("retCode: %u\n", retCode);
	termination(retCode);
}
int main(int argc, char **argv)
{
	char *domain = nextArg();

	errorCase_m(
		lineExp(".<>", domain) ||
		lineExp("<>.", domain) ||
		lineExp("<>..<>", domain) ||
		!lineExp("<1,100,-.09AZaz>", domain) || // ファイル名に使用するため短め
//		!lineExp("<1,300,-.09AZaz>", domain) ||
		0,
		"不正なドメイン名"
		);

	CheckDyIP(domain);
}
