#pragma once

#include "C:\Factory\Common\all.h"
#include "C:\Factory\OpenSource\camellia.h"
#include "C:\Factory\OpenSource\md5.h"

void createResourceCluster(autoList_t *files, char *rootDir, autoBlock_t *rawKey, char *clusterFile);
void restoreResourceCluster(char *clusterFile, autoBlock_t *rawKey, char *destDir);
