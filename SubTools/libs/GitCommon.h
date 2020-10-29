#pragma once

#include "C:\Factory\Common\all.h"

void RemoveGitPaths(autoList_t *paths);
void EscapeUnusableResPath(char *path);
void PostGitMaskFile(char *file);
void PostGitIgnoreFile(char *file);
