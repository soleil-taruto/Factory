#pragma once

#include "C:\Factory\Common\all.h"

enum
{
	NEWLINE_CR = 1,
	NEWLINE_CRLF,
	NEWLINE_LF,
};

void CRLF_CheckFile(char *file, uint *p_num_cr, uint *p_num_crlf, uint *p_num_lf);
uint CRLF_GetTopFile(char *file);
void CRLF_ConvFile(char *rFile, char *wFile, char *newLine);
