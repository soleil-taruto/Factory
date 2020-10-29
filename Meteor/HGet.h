#pragma once

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Collabo.h"

void HGet_Reset(void);
void HGet_SetProxy_IE(void);
void HGet_SetProxy(char *host, uint portNo);
void HGet_SetConnectionTimeoutMillis(uint millis);
void HGet_SetTimeoutMillis(uint millis);
void HGet_SetNoTrafficTimeoutMillis(uint millis);
void HGet_SetUrl(char *url);
void HGet_SetHTTPVersion_10(void);
void HGet_AddHeaderField(char *name, char *value);
void HGet_SetBody(autoBlock_t *body);
void HGet_SetBody_BB(autoBlock_t *body1, autoBlock_t *body2);
void HGet_SetBody_BF(autoBlock_t *body1, char *body2File);
void HGet_SetBody_BFB(autoBlock_t *body1, char *body2File, autoBlock_t *body3);
void HGet_SetBody_F(char *bodyFile);
void HGet_SetBody_FB(char *body1File, autoBlock_t *body2);
void HGet_SetResBodySizeMax(uint64 resBodySizeMax);
int HGet_Head(void);
int HGet_GetOrPost(void);
uint HGet_GetResHeaderFieldCount(void);
char *HGet_GetResHeaderFieldName(uint index);
char *HGet_GetResHeaderFieldValue(uint index);
void HGet_MvResBodyFile(char *destFile);
autoBlock_t *HGet_GetResBody(void);
