/*
	���s�ƃ^�u�͏����ł��Ȃ��̂� sqlcmd �� -k �I�v�V������t���ĂˁB
*/

#include "RedirSqlTableReader.h"

int RSTR_NoHeaderMode;

autoList_t *RSTR_SecondLineToColSpans(char *line)
{
	autoList_t *colSpans = newList();
	char *p;
	uint count = 0;

	for (p = line; ; p++)
	{
		if (*p == '-')
		{
			count++;
		}
		else if (*p == ' ' || *p == '\0')
		{
			errorCase(!count); // ���[���͍l�����Ȃ��B
			addElement(colSpans, count);

			if (*p == '\0')
			{
				break;
			}
			count = 0;
		}
		else
		{
			error(); // ? '-', ' ' �ȊO�̕���
		}
	}
	return colSpans; // ������ (���� 1 �ȏ� && �e�v�f 1 �ȏ�) �͕ۏႳ���B
}
autoList_t *RSTR_LineToValues(char *line, autoList_t *colSpans, int asColName) // line ��j�󂷂�B
{
	uint cs;
	uint index;
	char *p = line;
	autoList_t *values = newList();

	if (!isJLine(line, 1, 0, 0, 1)) // �^�u�̖��� CP-932 �ł��邱�ƁB
	{
		line2JLine(line, 1, 0, 0, 1);
		cout("�����R�[�h�̕␳���s���܂����B\n");
	}
	foreach (colSpans, cs, index)
	{
		char *q = p;

		while (cs)
		{
			errorCase(!*p); // �s���r�؂ꂽ�B
			p = mbsNext(p);
			cs--;
		}
		if (index + 1 < getCount(colSpans)) // ? ��ŏI�J����
		{
			errorCase(*p != ' '); // �J�����̊Ԃ��󔒂ł͂Ȃ��B
			*p = '\0';
			p++;
		}
		else
		{
			errorCase(*p != '\0'); // �s���܂��I���Ȃ��B
		}
		trimEdge(q, ' ');

		if (asColName)
		{
			errorCase(asColName && *q == '\0'); // �J�������ɋ󕶎���͖����͂��B
			errorCase(findLine(values, q) < index); // �J�������̏d����F�߂Ȃ��B
		}
		addElement(values, (uint)strx(q));
	}
	return values;
}
void RedirSqlTableReader(char *file, FILE *outStrm, void (*writeColNames_x)(FILE *outStrm, autoList_t *colNames), void (*writeRow_x)(FILE *outStrm, autoList_t *row))
{
	FILE *fp = fileOpen(file, "rt");
	char *line;
	char *line2;
	autoList_t *colSpans;

	// �`���̃S�~��ǂݔ�΂��B
	{
		uint irow;

		for (irow = 0; ; irow++)
		{
			line = readLine(fp);
			errorCase(!line);

			if (lineExp("< ->", line) &&
				lineExp("-<>", line) &&
				lineExp("<>-", line) &&
				!lineExp("<>  <>", line)
				)
				break;

			memFree(line);
		}
		fileSeek(fp, SEEK_SET, 0);

		for (; 1 < irow; irow--) // (irow - 1) ���s�v�ȍs��
		{
			line = readLine(fp);
			errorCase(!line);
			memFree(line);
		}
	}

	line = readLine(fp);
	errorCase(!line);
	line2 = readLine(fp);
	errorCase(!line2);

	colSpans = RSTR_SecondLineToColSpans(line2);
	writeColNames_x(outStrm, RSTR_LineToValues(line, colSpans, 1));

	memFree(line);
	memFree(line2);

	for (; ; )
	{
		line = readLine(fp);
		errorCase(!line);

		if (!*line)
		{
			break;
		}
		writeRow_x(outStrm, RSTR_LineToValues(line, colSpans, 0));
		memFree(line);
	}
	releaseAutoList(colSpans);

	line = readLine(fp); // '(999 �s��������܂���)' �s
	errorCase(!line);
	errorCase(!lineExp("(<>)", line)); // �p��Ƃ����邩������Ȃ��̂� "()" �����`�F�b�N
	memFree(line);

	// �����̃S�~�𖳎�����B
	/*
	line = readLine(fp); // EOF
	errorCase(line);
	*/

	fileClose(fp);
}
static void TLF_WriteColNames_x(FILE *fp, autoList_t *row)
{
	writeLines2Stream_x(fp, row);
	writeChar(fp, '\n'); // �J���������X�g�̏I�[��������s (�J�������ɋ󕶎���͖���)
}
static void WriteHeaderNoop(FILE *fp, autoList_t *row)
{
	noop();
}
void RSTR_ToLinearFile(char *rfile, char *wfile)
{
	FILE *fp = fileOpen(wfile, "wt");

	RedirSqlTableReader(rfile, fp, RSTR_NoHeaderMode ? WriteHeaderNoop : TLF_WriteColNames_x, writeLines2Stream_x);
	fileClose(fp);
}
void RSTR_ToCSVFile(char *rfile, char *wfile)
{
	FILE *fp = fileOpen(wfile, "wt");

	RedirSqlTableReader(rfile, fp, RSTR_NoHeaderMode ? WriteHeaderNoop : RowToCSVStream_x, RowToCSVStream_x);
	fileClose(fp);
}
