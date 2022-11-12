/*
	使い方は ..\mail\mailForward.c と同じ
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Sequence.h"
#include "C:\Factory\OpenSource\mt19937.h"
#include "Common.h"

#define STOP_EV_NAME "{aa9766d4-2ae1-40fb-b721-a3bd808561db}"

/*
	BIGLOBE が、送受信ともに、添付ファイルを含めて1通あたり100MBなので、マージン入れて 108 MB とした。@ 2020.8.7
*/
#define MAILSIZEMAX 108000000 // 108 MB

static char *PopServer;
static uint PopPortno = 995;
static char *SmtpServer;
static uint SmtpPortno = 465;
static char *UserName;
static char *Passphrase;
static char *SelfMailAddress;
static autoList_t *GroupList;
static autoList_t *GroupNameList;
static autoList_t *UnreturnMemberList;
static autoList_t *SendOnlyMemberList;
static char *CounterFileBase;
static int RecvAndDeleteMode = 1;

static void DoSendTextMail(char *mailAddressTo, char *subject, autoList_t *bodyLines)
{
	char *prmFile = makeTempPath(NULL);
	FILE *prmFp;
	char *bodyLine;
	uint bodyLineIndex;

	LOGPOS();

	prmFp = fileOpen(prmFile, "wt");

	writeLine(prmFp, "/S");
	writeLine(prmFp, asLine(SmtpServer));
	writeLine(prmFp, "/P");
	writeLine_x(prmFp, xcout("%u", SmtpPortno));
	writeLine(prmFp, "/U");
	writeLine(prmFp, asLine(UserName));
	writeLine(prmFp, asLine(Passphrase));
	writeLine(prmFp, "/MF");
	writeLine(prmFp, asLine(SelfMailAddress));
	writeLine(prmFp, "/MT");
	writeLine(prmFp, asLine(mailAddressTo));
	writeLine(prmFp, "/T");
	writeLine(prmFp, asLine(subject));

	foreach (bodyLines, bodyLine, bodyLineIndex)
	{
		writeLine(prmFp, "/L");
		writeLine(prmFp, asLine(bodyLine));
	}
	fileClose(prmFp);

	coExecute_x(xcout("C:\\Factory\\SubTools\\mail\\SendTextMail.exe //R \"%s\"", prmFile));

	removeFile(prmFile);
	memFree(prmFile);

	LOGPOS();
}
static char *GetCounterFile(char *groupName)
{
	static char *file;
	memFree(file);
	return file = xcout("%s[%s].txt", CounterFileBase, groupName);
}
static uint GetCounter(char *groupName)
{
	uint counter = 1; // 初期値

	if (existFile(GetCounterFile(groupName)))
	{
		char *line = readFirstLine(GetCounterFile(groupName));

		if (lineExp("<1,9,09>", line)) // 10億-1でカンスト
			counter = toValue(line);

		memFree(line);
	}
	cout("Counter: %u\n", counter);
	return counter;
}
static uint NextCounter(char *groupName)
{
	uint counter = GetCounter(groupName);

	writeOneLine_cx(GetCounterFile(groupName), xcout("%u", counter + 1));
	return counter;
}
static char *MakeSubjectFrom(char *groupName, char *mailFrom, uint counter)
{
	char *name = strx(mailFrom);
	char *subject;

	strchrEnd(name, '@')[0] = '\0';
	subject = xcout("[%s]%s(%u)", groupName, name, counter);
	memFree(name);
	return subject;
}
static char *MakeDateField(void)
{
	stampData_t tmpsd = *getStampDataTime(0L);

	return xcout(
		"%s, %u %s %04u %02u:%02u:%02u +0900"
		,getEWeekDay(tmpsd.weekday)
		,tmpsd.day
		,getEMonth(tmpsd.month)
		,tmpsd.year
		,tmpsd.hour
		,tmpsd.minute
		,tmpsd.second
		);
}
static char *ToFairMailAddress(char *mailAddress) // ret: strr(mailAddress)
{
	char *p = strrchr(mailAddress, '<'); // 最後の '<'

	if (p)
	{
		char *q = strchr(++p, '>');

		if (q)
		{
			char *tmp = strxl(p, (uint)q - (uint)p);

			memFree(mailAddress);
			mailAddress = tmp;
		}
	}
	ucTrim(mailAddress);
	return mailAddress;
}
static int IsKnownMessageId(void) // ret: ? 既知のメール
{
	static char *lastMessageId;
	char *messageId = MP_GetHeaderValue("Message-Id");

	if (!messageId) // ? Message-Id 無し
	{
		cout("メッセージIDなし\n");
		return 0;
	}
	line2JLine(messageId, 1, 0, 0, 0); // 表示するため

	cout("MID.1: %s\n", messageId);

	if (!lastMessageId)
	{
		cout("初回メール\n");
		lastMessageId = messageId;
		return 0;
	}
	cout("MID.2: %s\n", lastMessageId);

	if (!strcmp(messageId, lastMessageId))
	{
		cout("前回のメッセージIDと同じ -> 既知のメール\n");
		memFree(messageId);
		return 1;
	}
	cout("前回のメッセージIDと違う -> 新しいメール\n");
	memFree(lastMessageId);
	lastMessageId = messageId;
	return 0;
}

static int DistributeOne_ErrorFlag;

static void DistributeOne(char *groupName, char *memberFrom, char *memberTo, uint counter) // 引数は全て安全(書式を満たす文字列か数値)である。
{
	autoBlock_t *mail = newBlock();
	char *mimeVersion = MP_GetHeaderValue("MIME-Version");
	char *contentType = MP_GetHeaderValue("Content-Type");
	char *contentTransferEncoding = MP_GetHeaderValue("Content-Transfer-Encoding");

	cout("D1.M %u bytes\n", getSize(c_MP_GetBody()));
	cout("D1.G [%s]\n", groupName);
	cout("D1.< [%s]\n", memberFrom);
	cout("D1.> [%s]\n", memberTo);
	cout("D1.C %u\n", counter);

	if (!contentTransferEncoding)
		contentTransferEncoding = strx("7bit"); // Content-Transfer-Encoding のデフォルトは "7bit", 強制的に指定している理由は不明 (..\mail\mailForward.c に倣った)

	ab_addLine(mail, "Date: ");
	ab_addLine_x(mail, MakeDateField());
	ab_addLine(mail, "\r\n");

	ab_addLine(mail, "From: ");
	ab_addLine(mail, SelfMailAddress);
	ab_addLine(mail, "\r\n");

	ab_addLine(mail, "To: ");
	ab_addLine(mail, memberTo);
	ab_addLine(mail, "\r\n");

	ab_addLine(mail, "Subject: ");
	ab_addLine_x(mail, MakeSubjectFrom(groupName, memberFrom, counter));
	ab_addLine(mail, "\r\n");

	ab_addLine(mail, "Message-Id: ");
	ab_addLine_x(mail, MakeMailMessageID(SelfMailAddress));
	ab_addLine(mail, "\r\n");

	if (mimeVersion)
	{
		ab_addLine(mail, "MIME-Version: ");
		ab_addLine(mail, mimeVersion);
		ab_addLine(mail, "\r\n");
	}
	if (contentType)
	{
		ab_addLine(mail, "Content-Type: ");
		ab_addLine(mail, contentType);
		ab_addLine(mail, "\r\n");
	}
	ab_addLine(mail, "Content-Transfer-Encoding: ");
	ab_addLine(mail, contentTransferEncoding);
	ab_addLine(mail, "\r\n");

	ab_addLine(mail, "X-Mailer: ");
	ab_addLine(mail, "mf-S");
	ab_addLine(mail, "\r\n");

	ab_addLine(mail, "\r\n"); // ヘッダ終端の空行

	ab_addBytes(mail, c_MP_GetBody());

	PrintMailEntity(mail);

	SendMail(SmtpServer, SmtpPortno, UserName, Passphrase, SelfMailAddress, memberTo, mail);

	DistributeOne_ErrorFlag |= SendMailLastErrorFlag;
	cout("DistributeOne_ErrorFlag: %d (%d)\n", DistributeOne_ErrorFlag, SendMailLastErrorFlag);

	releaseAutoBlock(mail);
	memFree(mimeVersion);
	memFree(contentType);
	memFree(contentTransferEncoding);
}
static void Distribute(autoList_t *memberList, char *groupName, char *mailFrom) // 引数は全て安全(書式を満たす文字列)である。
{
	char *memberFrom;
	char *member;
	uint index;
	int unreturn;
	uint counter;
	autoList_t *shuffledMemberList;

	cout("D.M %u bytes\n", getSize(c_MP_GetBody()));

	foreach (memberList, member, index)
		cout("D.B [%s]\n", member);

	cout("D.G [%s]\n", groupName);
	cout("D.< [%s]\n", mailFrom);

#if 1
	memberFrom = (char *)refElement(memberList, findLineComp(memberList, mailFrom, strcmp));
#else // same
	memberFrom = NULL;

	foreach (memberList, member, index)
	{
		if (!strcmp(mailFrom, member))
		{
			memberFrom = member;
			break;
		}
	}
#endif
	errorCase(!memberFrom); // ? not found
	unreturn = findLine(UnreturnMemberList, memberFrom) < getCount(UnreturnMemberList); // ? 'memberFrom' is unreturn member

	counter = NextCounter(groupName);

	shuffledMemberList = copyAutoList(memberList);
	shuffle(shuffledMemberList);

	DistributeOne_ErrorFlag = 0;

	foreach (shuffledMemberList, member, index)
	{
		int sendonly = findLine(SendOnlyMemberList, member) < getCount(SendOnlyMemberList); // ? 'member' is sendonly member

		cout("member: %s\n", member);
		cout("unreturn: %d\n", unreturn);
		cout("sendonly: %d\n", sendonly);

		if (unreturn && member == memberFrom)
		{
			cout("■折り返し拒否メンバーなので飛ばす。\n");
		}
		else if (sendonly)
		{
			cout("■送信オンリーメンバーなので飛ばす。\n");
		}
		else
		{
			// 待ちは mutexUnlock() で行うことにした。
//			cout("★即返信・連続送信すると失敗することがあるっぽいのでちょっと待つ。\n");
//			coSleep(3000);

			DistributeOne(groupName, memberFrom, member, counter);
		}
	}
	releaseAutoList(shuffledMemberList);

	if (DistributeOne_ErrorFlag)
	{
		autoList_t *bodyLines = newList();
		char *fromField = MP_GetHeaderValue("From");
		char *dateField = MP_GetHeaderValue("Date");
		char *messageId = MP_GetHeaderValue("Message-Id");

		LOGPOS();

		if (!fromField)
			fromField = strx("<NONE>");

		if (!dateField)
			dateField = strx("<NONE>");

		if (!messageId)
			messageId = strx("<NONE>");

		toAsciiLine(fromField, 0, 0, 1); // 送信テキストに使用するため
		toAsciiLine(dateField, 0, 0, 1); // 送信テキストに使用するため
		toAsciiLine(messageId, 0, 0, 1); // 送信テキストに使用するため

		addElement(bodyLines, (uint)strx("メールの配信中にエラーが発生しました。"));
		addElement(bodyLines, (uint)strx("添付ファイルや、埋め込まれたコンテンツに問題があるかもしれません。"));
		addElement(bodyLines, (uint)strx("-- メールの情報"));
		addElement(bodyLines, (uint)xcout("From: %s", fromField));
		addElement(bodyLines, (uint)xcout("Date: %s", dateField));
		addElement(bodyLines, (uint)xcout("Message-Id: %s", messageId));
		addElement(bodyLines, (uint)xcout("Counter: %u", counter));
		addElement(bodyLines, (uint)strx("--"));
		addElement(bodyLines, (uint)strx("このメールは「エラーになったメールの送信元」にのみ送信しています。"));

		DoSendTextMail(mailFrom, "メール配信エラーのお知らせ", bodyLines);

		memFree(dateField);
		memFree(messageId);
		releaseDim(bodyLines, 1);

		LOGPOS();
	}
}
static void RecvEvent(void)
{
	char *mailFrom = MP_GetHeaderValue("From");

	if (!mailFrom)
	{
		cout("No mailFrom!\n");
		return;
	}
	mailFrom = ToFairMailAddress(mailFrom);

	{
		autoList_t *indexes = newList();
		autoList_t *memberList;
		uint index;
		uint index_index;
		char *mail_myself = NULL;

		foreach (GroupList, memberList, index)
		{
			char *member;
			uint member_index;

			foreach (memberList, member, member_index)
			{
				if (!strcmp(mailFrom, member))
				{
					addElement(indexes, index);
					mail_myself = member;
				}
			}
		}

		{
			char *subject = MP_GetHeaderValue("Subject");

			if (subject)
			{
				foreach (indexes, index, index_index)
				{
					char *groupPtn = xcout("[%s]", getLine(GroupNameList, index));

					if (mbs_stristr(subject, groupPtn)) // ? ターゲット発見
					{
						setCount(indexes, 1);
						setElement(indexes, 0, index);
						memFree(groupPtn);
						break;
					}
					memFree(groupPtn);
				}
				memFree(subject);
			}
		}

		if (2 <= getCount(indexes))
		{
			char *subject = MP_GetHeaderValue("Subject");

			if (!subject || !mbs_stristr(subject, "[]")) // ? 空のグループパターンが無い
			{
				errorCase(!mail_myself);
				DistributeOne_ErrorFlag = 0;
				DistributeOne("", mail_myself, mail_myself, 0);
				setCount(indexes, 0);
			}
			memFree(subject);
		}

		foreach (indexes, index, index_index)
		{
			Distribute(getList(GroupList, index), getLine(GroupNameList, index), mailFrom);
		}
		releaseAutoList(indexes);
	}

	memFree(mailFrom);
}
static void RecvLoop(void)
{
	uint stopEv = eventOpen(STOP_EV_NAME);
	uint waitSec = IMAX;

	for (; ; )
	{
		autoList_t *mailList = GetMailList(PopServer, PopPortno, UserName, Passphrase);
		int mailRecved = 0;
		uint index;

		if (getCount(mailList))
		{
			uint mailSize = getElement(mailList, 0);
			int del = 1; // 存在するけど受信できないメールがあった場合それを削除するために、デフォルトで 1 (削除)

			if (mailSize <= MAILSIZEMAX)
			{
				autoBlock_t *mail = RecvMail(PopServer, PopPortno, UserName, Passphrase, 1, MAILSIZEMAX);

				del = RecvAndDeleteMode;

				PrintMailEntity(mail);
				MailParser(mail);
				LOGPOS();

				if (!IsKnownMessageId())
					RecvEvent();
				else
					cout("既知のメールであるため無視します。\n");

				LOGPOS();
				MP_Clear();
				releaseAutoBlock(mail);
			}
			if (del)
				DeleteMail(PopServer, PopPortno, UserName, Passphrase, 1);

			mailRecved = 1;
		}
		releaseAutoList(mailList);

		if (mailRecved) // ? 何かメールを受信した。
			waitSec = 10;
		else
			waitSec++;

		m_minim(waitSec, 100);

		cout("waitSec: %u\n", waitSec);

		for (index = 0; index < waitSec; index += 3)
			if (checkKey(0x1b) || handleWaitForMillis(stopEv, 3000))
				goto endLoop;

		mt19937_rnd32(); // 乱数のカウンタを回す。
	}
endLoop:
	handleClose(stopEv);
}
int main(int argc, char **argv)
{
	mt19937_init();

	GroupList = newList();
	UnreturnMemberList = newList();
	SendOnlyMemberList = newList();

readArgs:
	if (argIs("/S"))
	{
		LOGPOS();
		eventWakeup(STOP_EV_NAME);
		return;
	}
	if (argIs("/PD")) // Pop server Domain
	{
		PopServer = nextArg();
		goto readArgs;
	}
	if (argIs("/PP")) // Pop server Port number
	{
		PopPortno = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/SD")) // Smtp server Domain
	{
		SmtpServer = nextArg();
		goto readArgs;
	}
	if (argIs("/SP")) // Smtp server Port number
	{
		SmtpPortno = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/U")) // User name
	{
		UserName = nextArg();
		goto readArgs;
	}
	if (argIs("/P")) // Passphrase
	{
		Passphrase = nextArg();
		goto readArgs;
	}
	if (argIs("/M")) // self Mail address
	{
		SelfMailAddress = nextArg();
		goto readArgs;
	}
	if (argIs("/F")) // members File
	{
		LOGPOS();
		addElement(GroupList, (uint)readResourceLines(nextArg()));
		goto readArgs;
	}
	if (argIs("/G")) // Groups file
	{
		LOGPOS();
		addElements_x(GroupList, readResourceFilesLines(nextArg()));
		goto readArgs;
	}
	if (argIs("/N")) // group Names file
	{
		LOGPOS();
		GroupNameList = readResourceLines(nextArg());
		goto readArgs;
	}
	if (argIs("/UR")) // Un-Return members file
	{
		LOGPOS();
		addElements_x(UnreturnMemberList, readResourceLines(nextArg()));
		goto readArgs;
	}
	if (argIs("/SO")) // Send Only members file
	{
		LOGPOS();
		addElements_x(SendOnlyMemberList, readResourceLines(nextArg()));
		goto readArgs;
	}
	if (argIs("/C")) // Counter file path-base
	{
		CounterFileBase = nextArg();
		goto readArgs;
	}
	if (argIs("/-D")) // no recv and Delete
	{
		LOGPOS();
		RecvAndDeleteMode = 0;
		goto readArgs;
	}

	LOGPOS();
	CheckMailServer(PopServer);
	CheckMailPortno(PopPortno);
	CheckMailServer(SmtpServer);
	CheckMailPortno(SmtpPortno);
	CheckMailUser(UserName);
	CheckMailPass(Passphrase);
	CheckMailAddress(SelfMailAddress);
	errorCase(!getCount(GroupList));
	LOGPOS();

	{
		autoList_t *memberList;
		uint memberList_index;

		foreach (GroupList, memberList, memberList_index)
		{
			char *member;
			uint index;

			errorCase(getCount(memberList) < 2);

			foreach (memberList, member, index)
			{
				CheckMailAddress(member);
			}
			errorCase(findPair(memberList, (sint (*)(uint, uint))strcmp));
		}
	}

	LOGPOS();
	errorCase(!GroupNameList);
	errorCase(getCount(GroupList) != getCount(GroupNameList));
	errorCase(m_isEmpty(CounterFileBase));
	// RecvAndDeleteMode
	LOGPOS();

	{
		char *groupName;
		uint index;

		foreach (GroupNameList, groupName, index)
		{
			errorCase(!lineExp("<1,9,09AZaz>", groupName));
		}
		errorCase(findPair(GroupNameList, (sint (*)(uint, uint))strcmp));
	}

	LOGPOS();

	{
		char *member;
		uint index;

		foreach (UnreturnMemberList, member, index)
		{
			CheckMailAddress(member);
		}
	}

	LOGPOS();

	{
		char *member;
		uint index;

		foreach (SendOnlyMemberList, member, index)
		{
			CheckMailAddress(member);
		}
	}

	LOGPOS();
	GetCounter("test"); // カウンタ取得テスト

	LOGPOS();
	cmdTitle("mailForward-S");
	RecvLoop();
	cout("\\e\n");
}
