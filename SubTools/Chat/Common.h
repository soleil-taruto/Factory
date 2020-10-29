#pragma once

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\URL.h"

extern char *UTP_Path;
extern int UTP_DownloadMode;
extern int UTP_HtmlMode;
extern int UTP_EndSlash;

char *URLToPath(char *url);
char *PathToURL(char *path);

char *LiteUrlEncoder(char *name);
