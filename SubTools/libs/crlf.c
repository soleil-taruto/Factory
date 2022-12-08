#include "crlf.h"

void CRLF_CheckFile(char *file, uint *p_num_cr, uint *p_num_crlf, uint *p_num_lf)
{
	FILE *fp = fileOpen(file, "rb");
	int backed = 0;
	int backedChr;
	int chr;
	uint num_cr = 0;
	uint num_crlf = 0;
	uint num_lf = 0;

	for (; ; )
	{
		if (backed)
		{
			backed = 0;
			chr = backedChr;
		}
		else
		{
			chr = readChar(fp);
		}

		if (chr == EOF)
			break;

		if (chr == '\r')
		{
			chr = readChar(fp);

			if (chr != '\n')
			{
				backed = 1;
				backedChr = chr;

				num_cr++;
			}
			else
			{
				num_crlf++;
			}
		}
		else if (chr == '\n')
		{
			num_lf++;
		}
	}
	fileClose(fp);

	if (p_num_cr)   *p_num_cr   = num_cr;
	if (p_num_crlf) *p_num_crlf = num_crlf;
	if (p_num_lf)   *p_num_lf   = num_lf;
}
uint CRLF_GetTopFile(char *file)
{
	uint num_cr;
	uint num_crlf;
	uint num_lf;

	CRLF_CheckFile(file, &num_cr, &num_crlf, &num_lf);

	if (num_cr < num_lf)
	{
		if (num_crlf < num_lf)
			return NEWLINE_LF;
	}
	else
	{
		if (num_crlf < num_cr)
			return NEWLINE_CR;
	}
	return NEWLINE_CRLF;
}
void CRLF_ConvFile(char *rFile, char *wFile, char *newLine) // newLine: "\r", "\r\n", "\n", etc.
{
	FILE *rfp = fileOpen(rFile, "rb");
	FILE *wfp = fileOpen(wFile, "wb");
	int backed = 0;
	int backedChr;
	int chr;

	for (; ; )
	{
		if (backed)
		{
			backed = 0;
			chr = backedChr;
		}
		else
		{
			chr = readChar(rfp);
		}

		if (chr == EOF)
			break;

		if (chr == '\r')
		{
			chr = readChar(rfp);

			if (chr != '\n')
			{
				backed = 1;
				backedChr = chr;
			}
			writeToken(wfp, newLine);
		}
		else if (chr == '\n')
		{
			writeToken(wfp, newLine);
		}
		else
		{
			writeChar(wfp, chr);
		}
	}
	fileClose(rfp);
	fileClose(wfp);
}
