/*
	mailForward.exe /PD POP3サーバー [/PP POP3サーバーのポート番号]
	                /SD SMTPサーバー [/SP SMTPサーバーのポート番号]
	                /U  メールアカウントのユーザー名
	                /P  メールアカウントのパスワード
	                /M  メールアカウントのメールアドレス
	                [/F メンバーリストファイル]...
	                [/G グループリストファイル]...
	                /N  グループ名リストファイル
	                /UR 折り返し拒否メンバーリストファイル
	                /SO 送信専用メンバーリストファイル
	                /C  カウンターファイルのベース名
					/-D

		起動する。
		エスケープキーの押下または「停止のリクエスト」により停止する。

		/-D ... 受信したメールの削除を行わない。
		        受信しただけで当該メールを削除するメールサーバーもある。

	mailForward.exe /S

		停止をリクエストする。

	----
	ファイル一覧

	メンバーリストファイル
		グループを構成するメンバーのメールアドレスのリストを改行区切りで記載する。
		同じグループ内でメールアドレスが重複してはならない。異なるグループ間での重複は可。
		読み込み方 -> readResourceLines();
		メールアドレスの書式は <1,300,@@__-.09AZaz> を想定する。大文字小文字を区別する。

	グループリストファイル
		メンバーリストファイルのリストを改行区切りで記載する。
		読み込み方 -> readResourceFilesLines();

	グループ名リストファイル
		グループ名のリストを改行区切りで記載する。
		グループ名の並びと個数は「メンバーリストファイル」の指定順に一致しなければならない。
		グループ名は重複してはならない。
		読み込み方 -> readResourceLines();
		グループ名の書式は <1,9,09AZaz> を想定する。大文字小文字を区別しない。

	折り返し拒否メンバーリストファイル
		自分のメールを自分自身に配信しないようにするメンバー(メールアドレス)のリストを改行区切りで記載する。
		グループを問わず「メンバーリストファイル」のメールアドレスとの完全一致によって有効になる。
		読み込み方 -> readResourceLines();
		メールアドレスの書式は <1,300,@@__-.09AZaz> を想定する。大文字小文字を区別する。

	送信専用メンバーリストファイル
		送信しかしないメンバー(メールアドレス)のリストを改行区切りで記載する。
		グループを問わず「メンバーリストファイル」のメールアドレスとの完全一致によって有効になる。
		読み込み方 -> readResourceLines();
		メールアドレスの書式は <1,300,@@__-.09AZaz> を想定する。大文字小文字を区別する。

	----
	メール受信時の動作

	1つのグループに属するメンバーからのメール

		そのグループ全員に配信する。

	複数のグループに属するメンバーからのメール

		件名の "[グループ名]" からグループを特定し、そのグループ全員に配信する。
		グループを特定できなかった場合は "[]" を件名にして送信者に送り返す。

	どのグループにも属さないメール

		破棄する。
*/

#include "pop3.h"
#include "smtp.h"
#include "tools.h"
#include "C:\Factory\Common\Options\Sequence.h"
#include "C:\Factory\OpenSource\mt19937.h"

#define STOP_EV_NAME "{c991a7e5-24bf-4ab0-a657-fc0a44827620}"

static char *PopServer;
static uint PopPortno = 110;
static char *SmtpServer;
static uint SmtpPortno = 25;
static char *PopUserName;
static char *PopPassphrase;
static char *SelfMailAddress;
static autoList_t *GroupList;
static autoList_t *GroupNameList;
static autoList_t *UnreturnMemberList;
static autoList_t *SendOnlyMemberList;
static char *CounterFileBase;
static int RecvAndDeleteMode = 1;

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

static void DistributeOne(autoList_t *mail, char *groupName, char *memberFrom, char *memberTo, uint counter) // mail 以外の引数は全て安全(書式を満たす文字列か数値)である。
{
	char *date;
	char *sendFrom;
	char *sendTo;
	char *subject;
	char *subjectFrom;
	char *messageID;
	char *mimeVersion;
	char *contentType;
	char *contentTransferEncoding;
	char *xMailer;
	autoList_t *sendData = newList();
	uint count;

	cout("D1.M %u lines\n", getCount(mail));
	cout("D1.G [%s]\n", groupName);
	cout("D1.< [%s]\n", memberFrom);
	cout("D1.> [%s]\n", memberTo);
	cout("D1.C %u\n", counter);

	date = GetMailHeader(mail, "Date");
	sendFrom = strx(memberFrom);
	sendTo = strx(memberTo);
	subject = GetMailHeader(mail, "Subject");
	subjectFrom = MakeSubjectFrom(groupName, memberFrom, counter);
	messageID = MakeMailMessageID(SelfMailAddress);
	mimeVersion = GetMailHeader(mail, "MIME-Version");
	contentType = GetMailHeader(mail, "Content-Type");
	contentTransferEncoding = GetMailHeader(mail, "Content-Transfer-Encoding");
	xMailer = strx("mf");

	// 日付を付け直す。
	{
		LOGPOS();
		memFree(date);
		date = MakeDateField();
		LOGPOS();
	}

	if (!contentTransferEncoding)
		contentTransferEncoding = strx("7bit"); // Content-Transfer-Encoding のデフォルトは "7bit", 強制的に指定している理由は不明

	if (
		!date ||
		!subject ||
		!mimeVersion ||
		!contentType ||
		!contentTransferEncoding
		)
	{
		cout(
			"Header Error! %u %u %u %u %u\n"
			,date
			,subject
			,mimeVersion
			,contentType
			,contentTransferEncoding
			);
		goto endfunc;
	}

	addElement(sendData, (uint)xcout("Date: %s", date));

	/*
		docomo は Reply-To を無視する。
		vodafone も送信元が携帯のものに対しては Reply-To を無視するようだ。
	*/
	if (
		// 注意: if else の処理を入れ替えた。
		/*
		strstr(memberTo, "@docomo.ne.jp") ||
		strstr(memberTo, ".vodafone.ne.jp") || // @?.vodafone.ne.jp
		strstr(memberTo, "@gmail.com")
		*/
//		strstr(memberTo, "@di.pdx.ne.jp") // del @ 2018.4.8
		0
		)
	{
		addElement(sendData, (uint)xcout("From: %s", memberFrom));
		addElement(sendData, (uint)xcout("To: %s", memberTo));
		addElement(sendData, (uint)xcout("Reply-To: %s", SelfMailAddress));
	}
	else
	{
		addElement(sendData, (uint)xcout("From: %s", SelfMailAddress));
		addElement(sendData, (uint)xcout("To: %s", memberTo));
	}
	addElement(sendData, (uint)xcout("Subject: %s", subjectFrom));
	addElement(sendData, (uint)xcout("Message-Id: %s", messageID));
	addElement(sendData, (uint)xcout("MIME-Version: %s", mimeVersion));
	addElement(sendData, (uint)xcout("Content-Type: %s", contentType));
	addElement(sendData, (uint)xcout("Content-Transfer-Encoding: %s", contentTransferEncoding));
	addElement(sendData, (uint)xcout("X-Mailer: %s", xMailer));
	addElement(sendData, (uint)strx(""));
	addLines_x(sendData, GetMailBody(mail));

//	sendMailEx2(SmtpServer, SmtpPortno, memberFrom, memberTo, sendData); // bug ? @ 2016.7.19
	sendMailEx2(SmtpServer, SmtpPortno, SelfMailAddress, memberTo, sendData);

endfunc:
	memFree(date);
	memFree(sendFrom);
	memFree(sendTo);
	memFree(subject);
	memFree(subjectFrom);
	memFree(messageID);
	memFree(mimeVersion);
	memFree(contentType);
	memFree(contentTransferEncoding);
	memFree(xMailer);
	releaseDim(sendData, 1);
}
static void Distribute(autoList_t *mail, autoList_t *memberList, char *groupName, char *mailFrom) // mail 以外の引数は全て安全(書式を満たす文字列)である。
{
	char *memberFrom;
	char *member;
	uint index;
	int unreturn;
	uint counter;
	autoList_t *shuffledMemberList;

	cout("D.M %u lines\n", getCount(mail));

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

			DistributeOne(mail, groupName, memberFrom, member, counter);
		}
	}
	releaseAutoList(shuffledMemberList);
}
static void RecvEvent(autoList_t *mail)
{
	char *mailFrom = GetMailHeader(mail, "From");

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
			char *subject = GetMailHeader(mail, "Subject");

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
			char *subject = GetMailHeader(mail, "Subject");

			if (!subject || !mbs_stristr(subject, "[]")) // ? 空のグループパターンが無い
			{
				errorCase(!mail_myself);
				DistributeOne(mail, "", mail_myself, mail_myself, 0);
				setCount(indexes, 0);
			}
			memFree(subject);
		}

		foreach (indexes, index, index_index)
		{
			Distribute(mail, getList(GroupList, index), getLine(GroupNameList, index), mailFrom);
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
		autoList_t *mails = mailRecv(PopServer, PopPortno, PopUserName, PopPassphrase, 3, 1024 * 1024 * 64, RecvAndDeleteMode);
		autoList_t *mail;
		uint index;

		foreach (mails, mail, index)
		{
			/*
				即返信すると破棄されることがある？
			*/
//			coSleep(3000); // moved @ 2016.6.28

			RecvEvent(mail);
		}
		releaseDim(mails, 2);

		if (index) // ? 何かメールを受信した。
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
		PopUserName = nextArg();
		goto readArgs;
	}
	if (argIs("/P")) // Passphrase
	{
		PopPassphrase = nextArg();
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
	errorCase(m_isEmpty(PopServer));
	errorCase(!PopPortno || 0xffff < PopPortno);
	errorCase(m_isEmpty(SmtpServer));
	errorCase(!SmtpPortno || 0xffff < SmtpPortno);
	errorCase(m_isEmpty(PopUserName));
	errorCase(m_isEmpty(PopPassphrase));
	errorCase(m_isEmpty(SelfMailAddress));
	errorCase(!getCount(GroupList));
	LOGPOS();

	{
		autoList_t *memberList;
		uint memberList_index;

		foreach (GroupList, memberList, memberList_index)
		{
			errorCase(getCount(memberList) < 2);
		}
	}

	LOGPOS();
	errorCase(!GroupNameList);
	errorCase(getCount(GroupList) != getCount(GroupNameList));
	errorCase(m_isEmpty(CounterFileBase));
	// RecvAndDeleteMode

	LOGPOS();
	GetCounter("test"); // カウンタ取得テスト

	LOGPOS();
	cmdTitle("mailForward");

	SockStartup();
	RecvLoop();
	SockCleanup();

	cout("\\e\n");
}
