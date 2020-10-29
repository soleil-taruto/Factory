#pragma once

#include "C:\Factory\Common\all.h"

void ReceptorLoop(
	char *startEventName,
	char *answerEventName,
	char *mutexName,
	char *prmFile,
	char *ansFile,
	int (*perform)(char *prmFile, char *ansFile),
	int (*idle)(void)
	);

void ReceptorLoopResFile(char *resFile, int (*perform)(char *prmFile, char *ansFile), int (*idle)(void));
