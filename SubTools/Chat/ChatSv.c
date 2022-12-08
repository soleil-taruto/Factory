/*
	ChatSv.exe [/S] [ポート番号]

		/S ... 停止する。起動直後だと止まらないこともある。ポート番号の前であること。
*/

#include "C:\Factory\Common\Options\SockServer.h"

#define TIMELINE_SIZEMAX 300000
#define REMARK_NUMMAX 1000

#define MEMBER_TOTALSIZEMAX 300000
#define MEMBER_NUMMAX 1000

static autoList_t *TimeLine; // { Remark_t * ... }
static uint TimeLineSize;

static autoList_t *Members;  // { Member_t * ... }
static uint MemberTotalSize;
static uint64 HeartbeatLoginLogoutSerial;

// ---- remark ----

typedef struct Remark_t
{
	char *Stamp;
	char *Message;
}
Remark_t;

static void Remark_Added(Remark_t *i)
{
	TimeLineSize += strlen(i->Stamp);
	TimeLineSize += strlen(i->Message);
}
static void ReleaseRemark(Remark_t *i)
{
	TimeLineSize -= strlen(i->Stamp);
	TimeLineSize -= strlen(i->Message);

	memFree(i->Stamp);
	memFree(i->Message);
	memFree(i);
}

// ---- member ----

typedef struct Member_st
{
	time_t LastTime;
	char *Ident;
	char *Message;
}
Member_t;

static void Member_Added(Member_t *i)
{
	MemberTotalSize += strlen(i->Ident);
	MemberTotalSize += strlen(i->Message);
}
static void Member_Removed(Member_t *i)
{
	MemberTotalSize -= strlen(i->Ident);
	MemberTotalSize -= strlen(i->Message);
}
static void ReleaseMember(Member_t *i)
{
	Member_Removed(i);

	memFree(i->Ident);
	memFree(i->Message);
	memFree(i);
}

// ----

static char *StampPlusOneSec(char *stamp)
{
#if 1
	uint64 t = toValue64(stamp);
	uint s;
	uint m;

	cout("SPOS_1: %s\n", stamp);

	s = (uint)(t % 100);
	t /= 100;
	m = (uint)(t % 100);
	t /= 100;

	s++; // plus one sec

	if (s == 60)
	{
		s = 0;
		m++;

		if (m == 60)
		{
			m = 0;
			t++;
		}
	}

	t *= 100;
	t += m;
	t *= 100;
	t += s;

	stamp = xcout("%I64u", t); // YYYYMMDD235959 -> YYYYMMDD240000 になるけど、いいや。

	cout("SPOS_2: %s\n", stamp);

	return stamp;
#else // old
	time_t t = compactStampToTime(stamp);

	cout("SPOS_1: %s\n", stamp);
	t++;
	stamp = makeCompactStamp(getStampDataTime(t));
	cout("SPOS_2: %s\n", stamp);
	return stamp;
#endif
}
static int Perform(char *prmFile, char *ansFile)
{
	char *text = readText_b(prmFile);
	autoList_t *lines;
	char *command;

	text = lineToJDoc_x(text, 1);
	lines = tokenize(text, '\n');
	command = refLine(lines, 0);

	cout("command: %s\n", command);

	if (!strcmp(command, "REMARK"))
	{
		char *stamp   = strx(refLine(lines, 1));
		char *message = strx(refLine(lines, 2));

		cout("REMARK %s:%s\n", stamp, message);

		while (REMARK_NUMMAX < getCount(TimeLine) || TIMELINE_SIZEMAX < TimeLineSize)
		{
			Remark_t *remark = (Remark_t *)desertElement(TimeLine, 0);

			ReleaseRemark(remark);
		}

		// 送信されたタイムスタンプにあまりズレが無ければ、それを採用する。
		{
			char *realStamp = makeCompactStamp(NULL);
			int okFlag = 0;

			if (lineExp("<14,09>", stamp))
			{
				time_t diff = _abs64(compactStampToTime(stamp) - compactStampToTime(realStamp));

				if (diff < 5)
					okFlag = 1;
			}
			if (!okFlag)
			{
				memFree(stamp);
				stamp = realStamp;
			}
			else
			{
				memFree(realStamp);
			}
		}
		if (getCount(TimeLine)) // 前の発言と同じ・より過去にならないように矯正する。
		{
			char *lastStamp = ((Remark_t *)getLastElement(TimeLine))->Stamp;
			char *availableStamp;

			availableStamp = StampPlusOneSec(lastStamp);

			if (strcmp(stamp, availableStamp) < 0) // ? stamp < availableStamp
			{
				memFree(stamp);
				stamp = strx(availableStamp);
			}
			memFree(availableStamp);
		}

		{
			Remark_t *remark = nb_(Remark_t);

			remark->Stamp = stamp;
			remark->Message = message;

			addElement(TimeLine, (uint)remark);
			Remark_Added(remark);
		}

		writeOneLineNoRet(ansFile, "REMARK_OK");
	}
	else if (!strcmp(command, "TIME-LINE"))
	{
		char *bgnStmp = refLine(lines, 1);
		char *endStmp = refLine(lines, 2);
		Remark_t *remark;
		uint remark_index;
		autoBlock_t *buff = newBlock();

		// extra data >

		ab_addLine_x(buff, xcout("%I64u\n", HeartbeatLoginLogoutSerial));

		// < extra data

		cout("TIME-LINE %s -> %s\n", bgnStmp, endStmp);

		foreach (TimeLine, remark, remark_index)
		{
			if (strcmp(bgnStmp, remark->Stamp) < 0 && strcmp(remark->Stamp, endStmp) < 0) // ? bgnStmp ～ endStmp, (bgnStmp, endStmp) は含まない。
			{
				cout("[%s] %s\n", remark->Stamp, remark->Message);

				ab_addLine(buff, remark->Stamp);
				ab_addLine(buff, "\n");
				ab_addLine(buff, remark->Message);
				ab_addLine(buff, "\n");
			}
		}
		cout("TIME-LINE END\n");
		writeBinary_cx(ansFile, buff);

//		memFree(bgnStmp); // del_dbg @ 2016.12.3
//		memFree(endStmp); // del_dbg @ 2016.12.3
	}
	else if (!strcmp(command, "HEARTBEAT"))
	{
		char *ident   = refLine(lines, 1);
		char *message = refLine(lines, 2);
		Member_t *member;
		uint index;
		uint currTime = now();
		autoBlock_t *buff = newBlock();

		foreach (Members, member, index)
			if (!strcmp(member->Ident, ident))
				break;

		if (member)
		{
			Member_Removed(member);

			if (strcmp(member->Message, message)) // ? メッセージが変更された。
			{
				cout("★★★ MESSAGE CHANGED [%s]\n", ident);

				HeartbeatLoginLogoutSerial++;
			}
			else
			{
				cout("☆☆☆ message not changed [%s]\n", ident);
			}

			memFree(member->Message);
			member->Message = strx(message);
		}
		else
		{
			cout("LOGIN MEMBER [%s]\n", ident);

			member = nb_(Member_t);
			member->Ident = strx(ident);
			member->Message = strx(message);
			addElement(Members, (uint)member);

			HeartbeatLoginLogoutSerial++;
		}
		member->LastTime = time(NULL);
		Member_Added(member);

		// ---- timeout ----

		foreach (Members, member, index)
		{
			if (member->LastTime + 300 < currTime) // ? timeout
			{
				cout("TIMEOUT MEMBER [%s]\n", member->Ident);

				ReleaseMember(member);
				setElement(Members, index, 0);

				HeartbeatLoginLogoutSerial++;
			}
		}
		removeZero(Members);

		// ---- overflow ----

		while (MEMBER_NUMMAX < getCount(Members) || MEMBER_TOTALSIZEMAX < MemberTotalSize)
		{
			uint oldestPos = 0;
			uint oldestTime = UINTMAX;

			foreach (Members, member, index)
			{
				if (member->LastTime < oldestTime)
				{
					oldestPos = index;
					oldestTime = member->LastTime;
				}
			}
			member = (Member_t *)getElement(Members, oldestPos);

			cout("OVERFLOW MEMBER [%s]\n", member->Ident);

			ReleaseMember(member);
			fastDesertElement(Members, oldestPos);

			HeartbeatLoginLogoutSerial++;
		}

		// ----

		cout("MEMBER BEGIN\n");

		foreach (Members, member, index)
		{
			cout("MEMBER [%s] T=%u\n", member->Ident, currTime - member->LastTime);

			ab_addLine(buff, member->Ident);
			ab_addLine(buff, "\n");
			ab_addLine(buff, member->Message);
			ab_addLine(buff, "\n");
		}
		cout("MEMBER END\n");

		writeBinary_cx(ansFile, buff);
	}
	else if (!strcmp(command, "LOGOUT"))
	{
		char *ident = refLine(lines, 1);
		Member_t *member;
		uint index;

		cout("LOGOUT MEMBER [%s]\n", ident);

		foreach (Members, member, index)
		{
			if (!strcmp(member->Ident, ident))
			{
				cout("LOGOUT OK\n");

				ReleaseMember(member);
				setElement(Members, index, 0);

				HeartbeatLoginLogoutSerial++;
			}
		}
		removeZero(Members);
		writeOneLineNoRet(ansFile, "LOGOUT_OK");
	}
	else if (!strcmp(command, "SERVER-TIME-DIFF"))
	{
		char *clStamp = refLine(lines, 1);
		char *svStamp = makeCompactStamp(NULL);
		time_t clTime;
		time_t svTime;
		char *ansLine;

		clTime = compactStampToTime(clStamp);
		svTime = compactStampToTime(svStamp);

		cout("clTime: %I64d\n", clTime);
		cout("svTime: %I64d\n", svTime);

		ansLine = xcout("%d", (sint)(svTime - clTime));

		cout("ansLine: %s\n", ansLine);

		writeOneLineNoRet(ansFile, ansLine);
		memFree(ansLine);
	}
	memFree(text);
	releaseDim(lines, 1);
	return 1;
}

#define STOP_EV_UUID "{3f4d1cc6-a7d8-4daf-8fc3-7b8b1f2fc52d}"

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
	uint portno = 59999;

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

	sockServer(Perform, portno, 3, 2000000, Idle);

	handleClose(StopEv);
}
