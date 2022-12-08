#include "BcpReader.h"

autoList_t *SqlBcpReader(char *bcpFile, int chrT, int chrR)
{
	autoList_t *table = newList();

	errorCase(m_isEmpty(bcpFile));
	errorCase(!m_isRange(chrT, 0x00, 0xff));
	errorCase(!m_isRange(chrR, 0x00, 0xff));
	errorCase(chrT == chrR);

	// �ǂݍ���
	{
		FILE *fp = fileOpen(bcpFile, "rb");
		autoList_t *row = newList();
		autoBlock_t *buff = newBlock();

		for (; ; )
		{
			int chr = readChar(fp);

			if (chr == EOF)
				break;

			if (chr == chrT || chr == chrR)
			{
				char *cell = unbindBlock2Line(buff);

				line2JLine(cell, 1, 0, 1, 1); // HACK: ���s�֎~

				addElement(row, (uint)cell);
				buff = newBlock();

				if (chr == chrR)
				{
					addElement(table, (uint)row);
					row = newList();
				}
			}
			else
			{
				addByte(buff, chr);
			}
		}
		fileClose(fp);

		errorCase(getCount(row)); // ? chrR �ŏI����Ă��Ȃ��B
		errorCase(getSize(buff)); // ? chrR �ŏI����Ă��Ȃ��B

		releaseAutoList(row);
		releaseAutoBlock(buff);
	}

	// check
	{
		if (getCount(table)) // ? �s���P�ȏ゠��B
		{
			uint colcnt = getCount(getList(table, 0));
			autoList_t * row;
			uint rowidx;
			char *cell;
			uint colidx;

			errorCase(colcnt < 1); // ? �񂪂P�������B

			foreach (table, row, rowidx)
			{
				errorCase(getCount(row) != colcnt); // ? �񐔂����ł͂Ȃ��B

				foreach (row, cell, colidx)
				{
					errorCase(cell == NULL); // ? cell == NULL
				}
			}
		}
	}
	return table;
}
