#include "Common.h"

void CheckMailServer(char *server)
{
	errorCase(!server);
	errorCase(!lineExp("<1,300,-.09AZaz>", server));
}
void CheckMailPortno(uint portno)
{
	errorCase(!m_isRange(portno, 1, 0xffff));
}
void CheckMailUser(char *user)
{
	errorCase(!user);
	errorCase(!lineExp("<1,100,@@..09AZaz>", user)); // 空白不可, 許可する文字を必要に応じて追加
}
void CheckMailPass(char *pass)
{
	errorCase(!pass);
	errorCase(!lineExp("<1,100,09AZaz>", pass)); // 空白不可, 許可する文字を必要に応じて追加
}
void CheckMailCommonParams(char *server, uint portno, char *user, char *pass)
{
	CheckMailServer(server);
	CheckMailPortno(portno);
	CheckMailUser(user);
	CheckMailPass(pass);
}
void CheckMailAddress(char *mailAddress)
{
	errorCase(!mailAddress);
	errorCase(!lineExp("<1,300,@@__-.09AZaz>", mailAddress));
}
