#include "C:\Factory\Common\all.h"
#include "..\GmailSend.h"

static char *GetText(char *prm)
{
	if (startsWith(prm, "**"))
		return strx(prm + 1);

	if (startsWith(prm, "*"))
		return readText_b(prm + 1);

	return strx(prm);
}
int main(int argc, char **argv)
{
	GS_Clear();

readArgs:
	if (argIs("/To"))
	{
		GS_AddTo(nextArg());
		goto readArgs;
	}
	if (argIs("/CC"))
	{
		GS_AddCC(nextArg());
		goto readArgs;
	}
	if (argIs("/BCC"))
	{
		GS_AddBCC(nextArg());
		goto readArgs;
	}
	if (argIs("/A"))
	{
		GS_AddAttachment(nextArg());
		goto readArgs;
	}
	if (argIs("/F"))
	{
		GS_SetFrom(nextArg());
		goto readArgs;
	}
	if (argIs("/S"))
	{
		GS_SetSubject(nextArg());
		goto readArgs;
	}
	if (argIs("/B"))
	{
		GS_SetBody_x(GetText(nextArg()));
		goto readArgs;
	}
	if (argIs("/U"))
	{
		GS_SetUser(nextArg());
		goto readArgs;
	}
	if (argIs("/P"))
	{
		GS_SetPassword(nextArg());
		goto readArgs;
	}
	if (argIs("/Host"))
	{
		GS_SetHost(nextArg());
		goto readArgs;
	}
	if (argIs("/Port"))
	{
		GS_SetPort(toValue(nextArg()));
		goto readArgs;
	}
	if (argIs("/-SSL"))
	{
		GS_SetSSLDisabled(1);
		goto readArgs;
	}

	termination(GS_Send() ? 0 : 1);
}
