uint mutexOpen(char *mutexName);
void handleClose(uint hdl);

int handleWaitForMillis(uint hdl, uint millis);
void handleWaitForever(uint hdl);
int mltHandleWaitForMillis(autoList_t *hdls, uint millis, uint *pIndex);
void mltHandleWaitForever(autoList_t *hdls, uint *pIndex);
void mutexRelease(uint hdl);

uint mutexLock(char *mutexName);
uint mutexTryLock(char *mutexName);
uint mutexTryProcLock(char *mutexName);
void mutexUnlock(uint hdl);

void mutex(void);
void unmutex(void);

uint eventOpen(char *eventName);
void eventSet(uint hdl);

void eventWakeupHandle(uint hdl);
void eventWakeup(char *eventName);
void eventSleep(uint hdl);

int collectEvents(uint hdl, uint millis);

// _x
uint mutexOpen_x(char *mutexName);
uint eventOpen_x(char *eventName);
