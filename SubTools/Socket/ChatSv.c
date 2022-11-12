/*
	★botsu ver

	ChatSv.exe [/S] [ポート番号]

		/S ... 停止する。起動直後だと止まらないこともある。ポート番号の前であること。
*/

#include "C:\Factory\Common\Options\SockServer.h"

#define USERNAME_LENMAX 60
#define MEMBER_MAX 100
#define REMARK_LENMAX 280
#define TIMELINE_MAX 1000

// ---- remark ----

typedef struct Remark_t
{
	char *Stamp;
	char *UserName;
	char *Message;
}
Remark_t;

static void ReleaseRemark(Remark_t *i)
{
	memFree(i->Stamp);
	memFree(i->UserName);
	memFree(i->Message);
	memFree(i);
}

// ---- member ----

typedef struct Member_st
{
	char *UserName;
	uint LastBeatTime;
}
Member_t;

static void ReleaseMember(Member_t *i)
{
	memFree(i->UserName);
	memFree(i);
}

// ----

static autoList_t *TimeLine; // { Remark_t * ... }
static autoList_t *Members;  // { Member_t * ... }

static int Perform(char *prmFile, char *ansFile)
{
	autoList_t *lines = readLines(prmFile);
	char *command;

	command = refLine(lines, 0);

	if (!strcmp(command, "REMARK"))
	{
		char *userName = refLine(lines, 1);
		char *message = refLine(lines, 2);

		userName = lineToJDocMax(userName, 0, USERNAME_LENMAX);
		message = lineToJDocMax(message, 0, REMARK_LENMAX);

		cout("REMARK (%s) %s\n", userName, message);

		while (TIMELINE_MAX < getCount(TimeLine)) // 2bs_while
		{
			Remark_t *remark = (Remark_t *)desertElement(TimeLine, 0);

			ReleaseRemark(remark);
		}

		{
			Remark_t *remark = nb_(Remark_t);

			remark->Stamp = makeCompactStamp(NULL);
			remark->UserName = userName;
			remark->Message = message;

			addElement(TimeLine, (uint)remark);
		}

		writeOneLineNoRet(ansFile, "REMARK_OK");

		// userName
		// message
	}
	else if (!strcmp(command, "TIME-LINE"))
	{
		char *bgnStmp = refLine(lines, 1);
		char *endStmp = refLine(lines, 2);
		Remark_t *remark;
		uint remark_index;
		autoBlock_t *buff = newBlock();

		bgnStmp = lineToJDocMax(bgnStmp, 0, 14);
		endStmp = lineToJDocMax(endStmp, 0, 14);

		cout("TIME-LINE %s -> %s\n", bgnStmp, endStmp);

		foreach (TimeLine, remark, remark_index)
		{
			if (strcmp(bgnStmp, remark->Stamp) < 0 && strcmp(remark->Stamp, endStmp) < 0) // ? bgnStmp ～ endStmp, (bgnStmp, endStmp) は含まない。
			{
				cout("+ %s (%s) %s\n", remark->Stamp, remark->UserName, remark->Message);

				ab_addLine(buff, remark->Stamp);
				ab_addLine(buff, "\t");
				ab_addLine(buff, remark->UserName);
				ab_addLine(buff, "\t");
				ab_addLine(buff, remark->Message);
				ab_addLine(buff, "\n");
			}
		}
		cout("TIME-LINE END\n");
		writeBinary_cx(ansFile, buff);

		memFree(bgnStmp);
		memFree(endStmp);
	}
	else if (!strcmp(command, "HEARTBEAT"))
	{
		char *userName = refLine(lines, 1);
		Member_t *member;
		uint index;
		uint currTime = now();
		autoBlock_t *buff = newBlock();

		userName = lineToJDocMax(userName, 0, USERNAME_LENMAX);

		foreach (Members, member, index)
			if (!strcmp(member->UserName, userName))
				break;

		if (!member)
		{
			cout("LOGIN MEMBER [%s]\n", userName);

			member = nb_(Member_t);
			member->UserName = strx(userName);
			addElement(Members, (uint)member);
		}
		member->LastBeatTime = currTime;

		// ---- timeout ----

		foreach (Members, member, index)
		{
			if (member->LastBeatTime + 60 < currTime) // ? timeout
			{
				cout("TIMEOUT MEMBER [%s]\n", member->UserName);

				ReleaseMember(member);
				setElement(Members, index, 0);
			}
		}
		removeZero(Members);

		// ---- overflow ----

		while (MEMBER_MAX < getCount(Members)) // 2bs_while
		{
			uint oldestPos = 0;
			uint oldestTime = UINTMAX;

			foreach (Members, member, index)
			{
				if (member->LastBeatTime < oldestTime)
				{
					oldestPos = index;
					oldestTime = member->LastBeatTime;
				}
			}
			member = (Member_t *)getElement(Members, oldestPos);

			cout("OVERFLOW MEMBER [%s] %u\n", member->UserName, oldestPos);

			ReleaseMember(member);
			fastDesertElement(Members, oldestPos);
		}

		// ----

		cout("MEMBER BEGIN\n");

		foreach (Members, member, index)
		{
			cout("MEMBER [%s] T=%u\n", member->UserName, currTime - member->LastBeatTime);

			ab_addLine(buff, member->UserName);
			ab_addLine(buff, "\n");
		}
		cout("MEMBER END\n");
		writeBinary_cx(ansFile, buff);
		memFree(userName);
	}
	else if (!strcmp(command, "LOGOUT"))
	{
		char *userName = refLine(lines, 1);
		Member_t *member;
		uint index;

		userName = lineToJDocMax(userName, 0, USERNAME_LENMAX);

		cout("LOGOUT MEMBER [%s]\n", userName);

		foreach (Members, member, index)
		{
			if (!strcmp(member->UserName, userName))
			{
				cout("LOGOUT OK\n");

				ReleaseMember(member);
				setElement(Members, index, 0);
			}
		}
		removeZero(Members);
		writeOneLineNoRet(ansFile, "LOGOUT_OK");
		memFree(userName);
	}
	releaseDim(lines, 1);
	return 1;
}

#define STOP_EV_UUID "{934bb764-b021-4335-9bb1-733552908fcf}"

static char *StopEvName;
static uint StopEv;

static int Idle(void)
{
	if (handleWaitForMillis(StopEv, 0))
		return 0;

	while (hasKey())
		if (getKey() == 0x1b)
			return 0;

	return 1;
}
int main(int argc, char **argv)
{
	int stopFlag = 0;
	uint portno = 59998;

	if (argIs("/S"))
		stopFlag = 1;

	if (hasArgs(1))
		portno = toValue(nextArg());

	StopEvName = xcout(STOP_EV_UUID "_%u", portno);

	if (stopFlag)
	{
		LOGPOS();
		eventWakeup(StopEvName);
		return;
	}
	StopEv = eventOpen(StopEvName);

	TimeLine = newList();
	Members = newList();

	sockServer(Perform, portno, 3, 1000, Idle);

	handleClose(StopEv);
}
