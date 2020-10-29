#include "smtps.h"

int SendMailLastErrorFlag; // ? 最後のメール送信は送信エラーだった。

void SendMail(char *smtpServer, uint portno, char *user, char *pass, char *fromMailAddress, char *toMailAddress, autoBlock_t *mail)
{
	char *upFile = makeTempPath(NULL);

	LOGPOS();

	CheckMailCommonParams(smtpServer, portno, user, pass);
	CheckMailAddress(fromMailAddress);
	CheckMailAddress(toMailAddress);
	errorCase(!mail);

	writeBinary(upFile, mail);

	mailLock();
	LOGPOS_T();

	coExecute(xcout(
		"curl smtps://%s:%u -u %s:%s --mail-from %s --mail-rcpt %s -T \"%s\""
		,smtpServer
		,portno
		,user
		,pass
		,fromMailAddress
		,toMailAddress
		,upFile
		));

	LOGPOS_T();
	cout("lastSystemRet: %d\n", lastSystemRet);
	SendMailLastErrorFlag = lastSystemRet != 0; // 終了コード 0 以外は送信エラーと見なす。
	mailUnlock();

	removeFile(upFile);
	memFree(upFile);

	LOGPOS();
}
