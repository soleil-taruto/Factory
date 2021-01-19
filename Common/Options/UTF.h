#pragma once

#include "C:\Factory\Common\all.h"

extern int UTF_BE;
extern int UTF_NoWriteBOM;
extern int UTF_UseJIS0208;

void UTF16ToSJISFile(char *rFile, char *wFile);
void SJISToUTF16File(char *rFile, char *wFile);
void UTF8ToUTF16File(char *rFile, char *wFile);
void UTF16ToUTF8File(char *rFile, char *wFIle);
void UTF8ToSJISFile(char *rFile, char *wFile);
void SJISToUTF8File(char *rFile, char *wFile);

char *UTF8ToSJISText(char *text);
char *SJISToUTF8Text(char *text);
char *UTF8ToSJISText_x(char *text);
char *SJISToUTF8Text_x(char *text);
