/*
	想定(必須)オプション
		Adapter
			/X 3000
			/TP
		crypTunnel
			/T 20
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Date2Day.h"
#include "C:\Factory\Labo\Socket\libs\Receptor.h"

static void KanaMask(char *str)
{
	char *p;

	for (p = str; *p; p = mbsNext(p))
	{
		switch (*p)
		{
		case 0x82: p[1] = 0xa0; break;
		case 0x83: p[1] = 0x41; break;
		}
	}
}

static uint64 GetSecond(uint64 t)
{
	uint y;
	uint m;
	uint d;
	uint h;
	uint i;
	uint s;

	t /= 100000;
	s = t % 100; t /= 100;
	i = t % 100; t /= 100;
	h = t % 100; t /= 100;
	d = t % 100; t /= 100;
	m = t % 100; t /= 100;
	y = t;

	return Date2Day(y, m, d) * 86400 + h * 3600 + i * 60 + s;
}

static uint64 LastTime;

static uint64 GetTime(void)
{
	stampData_t *i = getStampDataTime(0);
	uint64 t;

	errorCase(!isAllowStampData(i));

	t = i->year   * 1000000000000000ui64 +
		i->month  * 10000000000000ui64 +
		i->day    * 100000000000ui64 +
		i->hour   * 1000000000ui64 +
		i->minute * 10000000ui64 +
		i->second * 100000ui64;

	t = m_max(t, LastTime);
	t++;
	LastTime = t;
	return t;
}

// ----

#define NAMELENMAX 100
#define MESSAGELENMAX 2000
#define TWEETMAX 1000
#define TWEETMIN 900 // TWEETMAX を超えたとき、この数まで減らす。
#define RETTWEETMAX 10
#define DEFFIRSTTWEETMAX 30 // 初回アクセスと思しきときに返す発言数のデフォルト
#define TRAILERLENMAX 200
#define TRAILER_DLMTR ":"

typedef struct Tweet_st
{
	uint64 Time; // yyyyMMddhhmmssiiiii
	char *Name;
	char *Message;
	char *Trailer;
}
Tweet_t;

static Tweet_t *CreateTweet(char *name, char *message, autoList_t *trails)
{
	Tweet_t *i = (Tweet_t *)memAlloc(sizeof(Tweet_t));
	char *trailer;
	char *uiMessage;

	if (!name[0])
		name = "(N0)";
	else if (NAMELENMAX < strlen(name))
		name = "(NL)";

	if (!message[0])
		message = "(M0)";
	else if (MESSAGELENMAX < strlen(message))
		message = "(ML)";

	{
		static autoList_t *deftrails;

		if (!deftrails)
			deftrails = tokenize("TWE", '\1');

		if (!trails)
			trails = deftrails;

		if (strcmp(refLine(trails, getCount(trails) - 1), "TWE"))
		{
			message = "(ERROR:TRAILS-FORMAT)";
			trails = deftrails;
		}
	}

	i->Time = GetTime();
	i->Name = strx(name);
	i->Message = strx(message);
	i->Trailer = untokenize(trails, TRAILER_DLMTR);

	uiMessage = strx(i->Message);
	KanaMask(uiMessage);
	line2csym(uiMessage);

	cout("CreateTweet\n");
	cout("Time=%I64u\n", i->Time);
	cout("Name=%s\n", i->Name);
	cout("Message=%s\n", uiMessage);
	cout("Trailer=[%s]\n", i->Trailer);

	memFree(uiMessage);

	return i;
}
static void ReleaseTweet(Tweet_t *i)
{
	memFree(i->Name);
	memFree(i->Message);
	memFree(i->Trailer);
	memFree(i);
}

static autoList_t *TweetList;

// ----

#define MEMB_NAMELENMAX 80
#define MEMB_PASSLENMAX 100
#define MEMBERMAX 100

typedef struct Member_st
{
	char *Name;
	char *Passphrase;
	uint64 LastAccessTime;
}
Member_t;

static Member_t *CreateMemberLAT(char *name, char *passphrase, uint64 lat)
{
	Member_t *i = (Member_t *)memAlloc(sizeof(Member_t));

	if (!name[0])
		name = "(N0)";
	else if (MEMB_NAMELENMAX < strlen(name))
		name = "(NL)";

	if (!passphrase[0])
		passphrase = "(P0)";
	else if (MEMB_PASSLENMAX < strlen(passphrase))
		passphrase = "(PL)";

	i->Name = strx(name);
	i->Passphrase = strx(passphrase);
	i->LastAccessTime = lat;

	cout("CreateMember\n");
	cout("Name=%s\n", i->Name);
	cout("Passphrase=%s\n", i->Passphrase);
	cout("LastAccessTime=%I64u (LOG-IN-TIME)\n", i->LastAccessTime);

	return i;
}
static Member_t *CreateMember(char *name, char *passphrase)
{
	return CreateMemberLAT(name, passphrase, GetTime());
}
static void ReleaseMember(Member_t *i)
{
	memFree(i->Name);
	memFree(i->Passphrase);
	memFree(i);
}

static autoList_t *MemberList;

static Member_t *FindMember(char *name)
{
	Member_t *member;
	uint index;

	foreach (MemberList, member, index)
		if (!strcmp(name, member->Name))
			break;

	return member;
}

static uint64 ProwlerLastAccessTime;

static char *MemberNameFltr(char *name, char *passphrase) // ret: strr(name)
{
	Member_t *member = FindMember(name);

	if (member)
	{
		if (!strcmp(passphrase, member->Passphrase))
		{
			uint64 lastTime = member->LastAccessTime;
			uint64 currTime = GetTime();

			if (GetSecond(lastTime) + 70 < GetSecond(currTime)) // ? 過去１分１０秒以内にアクセス無し -> ログアウトしていたと見なす。
			{
				cout("LOG-IN %s (%s)\n", name, passphrase);
				cout("lastTime: %I64u\n", lastTime);
				cout("currTime: %I64u (LOG-IN-TIME)\n", currTime);
			}
			member->LastAccessTime = currTime;
		}
		else
		{
			name = addLine(name, "(偽)");
			ProwlerLastAccessTime = GetTime();
		}
	}
	else
	{
		if (MEMBERMAX <= getCount(MemberList))
		{
			cout("MEMBER OVER-FLOW! -> CLEAR\n");

			while (getCount(MemberList))
			{
				ReleaseMember((Member_t *)unaddElement(MemberList));
			}
		}
		addElement(MemberList, (uint)CreateMember(name, passphrase));
	}
	return name;
}

// ----

static FILE *ParamsFP;
static FILE *AnswerFP;

static char *ReadParam_Eof2Null(void)
{
	return readLine(ParamsFP);
}
static char *ReadParam(void)
{
	char *line = ReadParam_Eof2Null();

	if (!line)
		line = strx("");

	return line;
}
static char *ReadParam_MMS(uint minlen, uint maxlen, char *substituteParam)
{
	char *line = ReadParam();
	uint linelen;

	linelen = strlen(line);

	if (linelen < minlen || maxlen < linelen)
	{
		memFree(line);
		line = strx(substituteParam);
	}
	return line;
}
static void WriteAnsLine(char *line)
{
	writeLine(AnswerFP, line);
}
static void WriteAnsLine_x(char *line)
{
	WriteAnsLine(line);
	memFree(line);
}

static uint GetValueFromLines(autoList_t *lines, uint index, uint defaultValue, uint minval, uint maxval)
{
	uint value = toValue(refLine(lines, index));

	if (!value)
		value = DEFFIRSTTWEETMAX;

	m_range(value, minval, maxval);
	return value;
}
static int Perform(char *prmFile, char *ansFile)
{
	int retval = 0;
	char *command;

	ParamsFP = fileOpen(prmFile, "rt");
	AnswerFP = fileOpen(ansFile, "wt");

	command = ReadParam();

	if (!strcmp(command, "TW"))
	{
		char *name;
		char *passphrase;
		char *message;
		uint64 knownTimeLineTime;
		autoList_t *trails = newList();
		uint index;
		uint count;

		name = ReadParam_MMS(1, MEMB_NAMELENMAX, "(ERROR:NAME)");
		passphrase = ReadParam_MMS(1, MEMB_PASSLENMAX, "(ERROR:PASS)");
		message = ReadParam();
		knownTimeLineTime = toValue64_x(ReadParam());

		for (; ; )
		{
			char *trail = ReadParam_Eof2Null();

			if (!trail)
				break;

			addElement(trails, (uint)trail);
		}
		name = MemberNameFltr(name, passphrase);

		if (*message)
		{
			addElement(TweetList, (uint)CreateTweet(name, message, trails));

#if 0
			if (mbs_strstr(message, "ぬるぽ"))
				addElement(TweetList, (uint)CreateTweet(
					"(鯖)",
					"ガッ",
					NULL
					));
#endif

			if (TWEETMAX < getCount(TweetList))
			{
				reverseElements(TweetList); // 逆転

				while (TWEETMIN < getCount(TweetList))
				{
					ReleaseTweet((Tweet_t *)unaddElement(TweetList));
				}
				reverseElements(TweetList); // 復元
			}
		}
		for (index = getCount(TweetList); index; index--)
		{
			Tweet_t *tw = (Tweet_t *)getElement(TweetList, index - 1);

			if (tw->Time <= knownTimeLineTime)
			{
				break;
			}
		}
		if (index == 0) // ? knownTimeLineTime < 最古の発言の日時 -> 初回アクセスと見なす。
		{
			uint firstTweetMax = GetValueFromLines(trails, 2, DEFFIRSTTWEETMAX, 1, TWEETMAX);

			if (firstTweetMax < getCount(TweetList))
			{
				index = getCount(TweetList) - firstTweetMax;
			}
		}
		for (count = 0; index < getCount(TweetList) && count < RETTWEETMAX; index++, count++)
		{
			Tweet_t *tw = (Tweet_t *)getElement(TweetList, index);

			WriteAnsLine("TW");
			WriteAnsLine_x(xcout("%I64u", tw->Time));
			WriteAnsLine(tw->Name);
			WriteAnsLine(tw->Message);

			{
				autoList_t *trails = tokenize(tw->Trailer, TRAILER_DLMTR[0]);
				char *trail;
				uint trail_index;

				foreach (trails, trail, trail_index)
				{
					if (!*trail)
						trail = "(T0)";

					WriteAnsLine(trail);
				}
				releaseDim(trails, 1);
			}
		}

		{
			uint64 currTime = GetTime();
			Member_t *member;

			foreach (MemberList, member, index)
			{
				if (GetSecond(currTime) < GetSecond(member->LastAccessTime) + 60) // ? 過去１分未満にアクセス有り
				{
					WriteAnsLine("OL");
					WriteAnsLine(member->Name);
				}
			}
			if (GetSecond(currTime) < GetSecond(ProwlerLastAccessTime) + 60)
			{
				WriteAnsLine("OL");
				WriteAnsLine("(anonymous)");
			}
		}

		memFree(name);
		memFree(passphrase);
		memFree(message);
		releaseDim(trails, 1);

		retval = 1; // Successful
	}
	else if (!strcmp(command, "LO"))
	{
		char *name;
		char *passphrase;
		Member_t *member;

		name = ReadParam_MMS(1, MEMB_NAMELENMAX, "----");
		passphrase = ReadParam_MMS(1, MEMB_PASSLENMAX, "----");

		cout("LOG-OUT %s (%s)\n", name, passphrase);

		member = FindMember(name);

		if (member)
		{
			cout("FOUND MEMBER\n");

			if (!strcmp(passphrase, member->Passphrase))
			{
				cout("PASSPHRASE OK -> RESET LAST-ACCESS-TIME\n");
				member->LastAccessTime = 0;
			}
		}
		memFree(name);
		memFree(passphrase);

		cout("LOG-OUT-TIME: %I64u\n", GetTime());

		WriteAnsLine("BYE!");
		retval = 1; // Successful
	}
	else if (!strcmp(command, "XLO")) // ログアウトしない -> noop
	{
		char *name = ReadParam();
		cout("X-LOG-OUT %s\n", name);
		memFree(name);

		WriteAnsLine("BYE!");
		retval = 1; // Successful
	}
	memFree(command);

	fileClose(ParamsFP);
	fileClose(AnswerFP);

	return retval;
}

#define DUMPFILE "C:\\Factory\\tmp\\Hemachi_Server_Dump.txt"

static void OutputDump(void)
{
	FILE *fp = fileOpen(DUMPFILE, "wt");
	Tweet_t *tw;
	Member_t *member;
	uint index;
	char *uiMessage;

	cout("OutputDump Start...\n");

	foreach (TweetList, tw, index)
	{
		uiMessage = strx(tw->Message);
		KanaMask(uiMessage);
		line2csym(uiMessage);

		writeLine(fp, "[Tweet]");
		writeLine_x(fp, xcout("Time=%I64u", tw->Time));
		writeLine_x(fp, xcout("Name=%s", tw->Name));
		writeLine_x(fp, xcout("Message=%s", uiMessage));
		writeLine_x(fp, xcout("Trailer=[%s]", tw->Trailer));

		memFree(uiMessage);
	}
	foreach (MemberList, member, index)
	{
		writeLine(fp, "[Member]");
		writeLine_x(fp, xcout("Name=%s", member->Name));
		writeLine_x(fp, xcout("Passphrase=%s", member->Passphrase));
		writeLine_x(fp, xcout("LastAccessTime=%I64u", member->LastAccessTime));
	}
	writeLine_x(fp, xcout("ProwlerLastAccessTime=%I64u", ProwlerLastAccessTime));

	fileClose(fp);
	cout("OutputDump OK\n");

	execute("START " DUMPFILE);
	cout("ShowDump OK\n");
}
static int Idle(void)
{
	while (hasKey())
	{
		switch (getKey())
		{
		/*
		case '1':
			addElement(TweetList, (uint)CreateTweet(
				"(鯖)",
				"テスト",
				NULL
				));
			break;
		*/
		case 'D':
			OutputDump();
			break;

		case 0x1b:
			cout("End process.\n");
			return 0;

		default:
			cout("Press ESCAPE to end process. (D=OutputDump)\n");
			break;
		}
	}
	return 1;
}
int main(int argc, char **argv)
{
	remove(DUMPFILE);

	TweetList = newList();
	MemberList = newList();

	addElement(TweetList, (uint)CreateTweet(
		"(鯖)",
		"SMF", // "サーバーはメンテナンスから復帰しました。",
		NULL
		));

	addElement(MemberList, (uint)CreateMemberLAT(
		"(鯖)",
		"9999",
		0
		));

	cmdTitle("Hemachi Server");
	cout("Hemachi Server Start\n");

#if 1
	ReceptorLoopResFile(NULL, Perform, Idle);
#else // OLD
	ReceptorLoop(
		"cerulean.charlotte Hemachi Server start event object",
		"cerulean.charlotte Hemachi Server answer event object",
		"cerulean.charlotte Hemachi Server mutex object",
		"C:\\Factory\\tmp\\Hemachi_Server_Params.tmp",
		"C:\\Factory\\tmp\\Hemachi_Server_Answer.tmp",
		Perform,
		Idle
		);
#endif

	cout("Hemachi Server End\n");
}
