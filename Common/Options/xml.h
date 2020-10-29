#pragma once

#include "C:\Factory\Common\all.h"
#include "XNode.h"
#include "UTF.h"

XNode_t *readXMLFile(char *file);
void writeXMLFile(char *file, XNode_t *root);
void writeXMLFile_cx(char *file, XNode_t *root);
