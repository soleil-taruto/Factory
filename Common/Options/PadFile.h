#pragma once

#include "C:\Factory\Common\all.h"
#include "C:\Factory\OpenSource\md5.h"

void PadFile(char *file, autoBlock_t *footer);
int UnpadFile(char *file, autoBlock_t *footer);
void PadFileHash(char *file);
int UnpadFileHash(char *file);
void PadFileFooter(char *file, autoBlock_t *footer);
int UnpadFileFooter(char *file, autoBlock_t *footer);
void PadFileLine(char *file, char *footer);
int UnpadFileLine(char *file, char *footer);
void PadFileFooterLine(char *file, char *footer);
int UnpadFileFooterLine(char *file, char *footer);
void PadFile2(char *file, char *uniqueLabel);
int UnpadFile2(char *file, char *uniqueLabel);
