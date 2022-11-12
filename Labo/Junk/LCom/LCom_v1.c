#include "LCom_v1.h"

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

	mutex();
	writeOneLine(CombineEx(PREF_FILE_REQUEST, id), line);
	unmutex();

	eh = eventOpen(CombineEx(PREF_EVENT_RESPONSE, id));
	eventWakeup(CombineEx(PREF_EVENT_REQUEST, id));
	eventSleep(eh);

	mutex();
	line = readFirstLine(CombineEx(PREF_FILE_RESPONSE, id));
	unmutex();

	mutexUnlock(mh);
	return line;
}
void LComRecvLoop(char *(*funcRecvEvent)(char *), uint id)
{
	uint eh = eventOpen(CombineEx(PREF_EVENT_REQUEST, id));

	cout("Press ESCAPE key to exit LComRecvLoop().\n");

	while (!hasKey() || getKey() != 0x1b)
	{
		if (handleWaitForMillis(eh, 100))
		{
			char *line;
			char *resLine;

			mutex();
			line = readFirstLine(CombineEx(PREF_FILE_REQUEST, id));
			unmutex();

			resLine = funcRecvEvent(line);

			mutex();
			writeOneLine(CombineEx(PREF_FILE_RESPONSE, id), resLine);
			unmutex();

			eventWakeup(CombineEx(PREF_EVENT_RESPONSE, id));

			memFree(line);
			memFree(resLine);
		}
	}
	handleClose(eh);
}
