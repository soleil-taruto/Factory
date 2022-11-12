#include "libs\Server.h"

typedef enum Status_et
{
	CONNECTED,
	INPUT_USER,
	INPUT_PASSWORD,
	LOGGED_IN,
}
Status_t;

typedef struct Info_st
{
	Status_t Status;
	char *User;
}
Info_t;

uint CreateTelnetServerPerformInfo(void)
{
	Info_t *i = (Info_t *)memAlloc(sizeof(Info_t));

	i->Status = CONNECTED;
	i->User = NULL;

	return (uint)i;
}
void ReleaseTelnetServerPerformInfo(uint prm)
{
	Info_t *i = (Info_t *)prm;

	memFree(i->User);
	memFree(i);
}

#define DUMMY_PROMPT_FORMAT "C:/Users/%s.ADMINISTRATOR>"

char *TelnetServerPerform(char *inputLine, uint prm)
{
	Info_t *i = (Info_t *)prm;

	switch (i->Status)
	{
	case CONNECTED:
		i->Status = INPUT_USER;

		return strx(
			"Welcome to Masshirosoft Helmet Service\r\n"
			"\r\n"
			"login: "
			);

	case INPUT_USER:
		if (inputLine && *inputLine)
		{
			cout("[%s] %p user: %s\n", c_makeJStamp(NULL, 1), i, inputLine);

			i->Status = INPUT_PASSWORD;

			i->User = strx(inputLine);
			i->User = setStrLenRng(i->User, 4, 12, '_');
			line2csym(i->User);

			return strx("password: ");
		}
		break;

	case INPUT_PASSWORD:
		if (inputLine && *inputLine)
		{
			cout("[%s] %p password: %s\n", c_makeJStamp(NULL, 1), i, inputLine);

			i->Status = LOGGED_IN;

			return xcout(
				"\r\n"
				"*==============================================================================\r\n"
				"Masshirosoft Helmet Server.\r\n"
				"*==============================================================================\r\n"
				"\r\n"
				DUMMY_PROMPT_FORMAT
				,i->User
				);
		}
		break;

	case LOGGED_IN:
		if (inputLine)
		{
			if (*inputLine)
			{
				cout("[%s] %p command: %s\n", c_makeJStamp(NULL, 1), i, inputLine);

				if (
					!_stricmp(inputLine, "EXIT") ||
					!_stricmp(inputLine, "QUIT") ||
					!_stricmp(inputLine, "BYE")
					)
					return NULL;

				return xcout(
					"The filename, directory name, or volume label syntax is incorrect.\r\n"
					"\r\n"
					DUMMY_PROMPT_FORMAT
					,i->User
					);
			}
			return xcout(DUMMY_PROMPT_FORMAT, i->User);
		}
		break;

	default:
		error();
	}
	return strx("");
}
