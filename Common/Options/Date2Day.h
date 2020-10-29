#pragma once

#include "C:\Factory\Common\all.h"

uint64 Date2Day(uint y, uint m, uint d);
void Day2Date(uint64 day, uint *py, uint *pm, uint *pd);
int CheckDate(uint y, uint m, uint d);
uint GetEndOfMonth(uint y, uint m);
char *Date2JWeekday(uint y, uint m, uint d);

uint64 FileStampToMillis(uint64 stamp);
uint64 MillisToFileStamp(uint64 millis);
int IsFairFileStamp(uint64 stamp);

uint IDate2Day(uint date);
uint Day2IDate(uint day);
int IsFairIDate(uint date);

uint64 IDateTime2Sec(uint64 dateTime);
uint64 Sec2IDateTime(uint64 sec);
int IsFairIDateTime(uint64 dateTime);
