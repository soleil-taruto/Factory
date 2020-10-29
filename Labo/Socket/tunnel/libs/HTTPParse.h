#pragma once

#include "C:\Factory\Common\all.h"

typedef struct HttpDat_st
{
	autoList_t *Header;
	char *H_Request;
	autoList_t *H_Keys;
	autoList_t *H_Values;
	int Chunked;
	uint ContentLength;
	int Expect100Continue;
	autoBlock_t *Body;
	uint EndPos;
}
HttpDat_t;

extern HttpDat_t HttpDat;

int HTTPParse(autoBlock_t *buff);

HttpDat_t EjectHttpDat(void);
void UnejectHttpDat(HttpDat_t backup);

#define DEF_HTTP_DAT_FILE "HTTPParse_HttpDat.tmp"

void SaveHttpDat(char *file);
void LoadHttpDat(char *file);
