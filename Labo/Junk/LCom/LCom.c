#include "LCom.h"

#define PREF_MUTEX          "cerulean.charlotte Factory LCom mutex object "
#define PREF_EVENT_REQUEST  "cerulean.charlotte Factory LCom event object Request "
#define PREF_EVENT_RESPONSE "cerulean.charlotte Factory LCom event object Response "

#define PREF_FILE_REQUEST  "C:\\Factory\\tmp\\LCom_Request_"
#define PREF_FILE_RESPONSE "C:\\Factory\\tmp\\LCom_Response_"

static char *CombineEx(char *name, uint id)
{
	static char *nameid;

	memFree(nameid);
	return nameid = xcout("%s%u", name, id);
}

char *LComSendLine(char *line, uint id)
{
	uint mh = mutexLock(CombineEx(PREF_MUTEX, id));
	uint eh;

	eh = eventOpen(CombineEx(PREF_EVENT_RESPONSE, id));

	mutex();
	{
	rewrite:
		writeOneLine(CombineEx(PREF_FILE_REQUEST, id), line);
		remove(CombineEx(PREF_FILE_RESPONSE, id));

		for (; ; )
		{
			int eq;
			int es;

			unmutex();
			{
				eventWakeup(CombineEx(PREF_EVENT_REQUEST, id));
				collectEvents(eh, 2000);
			}
			mutex();

			eq = existFile(CombineEx(PREF_FILE_REQUEST, id));
			es = existFile(CombineEx(PREF_FILE_RESPONSE, id));

			if (!eq && es) // ? âûìöÉAÉä
			{
				break;
			}
			if (eq && !es) // ? ñ¢èàóù
			{
				// noop
			}
			else // ? ÇªÇÃëºÇÃïœÇ»èÛë‘
			{
				goto rewrite;
			}
		}
		line = readFirstLine(CombineEx(PREF_FILE_RESPONSE, id));
		removeFile(CombineEx(PREF_FILE_RESPONSE, id));
	}
	unmutex();

	handleClose(eh);
	mutexUnlock(mh);

	return line;
}
void LComRecvLoop(char *(*funcRecvEvent)(char *), uint id)
{
	uint eh = eventOpen(CombineEx(PREF_EVENT_REQUEST, id));

	mutex();
	remove(CombineEx(PREF_FILE_REQUEST, id));
	remove(CombineEx(PREF_FILE_RESPONSE, id));
	unmutex();

	cout("Press ESCAPE key to exit LComRecvLoop().\n");

	for (; ; )
	{
		while (hasKey())
			if (getKey() == 0x1b)
				goto endLoop;

		collectEvents(eh, 3000);

		mutex();
		{
			if (existFile(CombineEx(PREF_FILE_REQUEST, id)))
			{
				char *line = readFirstLine(CombineEx(PREF_FILE_REQUEST, id));
				char *resLine;

				removeFile(CombineEx(PREF_FILE_REQUEST, id));
				resLine = funcRecvEvent(line);
				writeOneLine(CombineEx(PREF_FILE_RESPONSE, id), resLine);

				memFree(line);
				memFree(resLine);
			}
			eventWakeup(CombineEx(PREF_EVENT_RESPONSE, id));
		}
		unmutex();
	}
	endLoop:

	handleClose(eh);
}
