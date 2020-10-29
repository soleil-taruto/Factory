#pragma once

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Collabo.h"

void GS_Clear(void);
void GS_AddTo(char *addr);
void GS_AddCC(char *addr);
void GS_AddBCC(char *addr);
void GS_AddAttachment(char *file);
void GS_SetFrom(char *addr);
void GS_SetSubject(char *line);
void GS_SetBody(char *text);
void GS_SetBody_x(char *text);
void GS_SetUser(char *line);
void GS_SetPassword(char *line);
void GS_SetHost(char *line);
void GS_SetPort(uint valPort);
void GS_SetSSLDisabled(int flag);

int GS_TrySend(void);
int GS_Send(void);
