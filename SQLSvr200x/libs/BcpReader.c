#include "BcpReader.h"

autoList_t *SqlBcpReader(char *bcpFile, int chrT, int chrR)
{
	autoList_t *table = newList();

	errorCase(m_isEmpty(bcpFile));
	errorCase(!m_isRange(chrT, 0x00, 0xff));
	errorCase(!m_isRange(chrR, 0x00, 0xff));
	errorCase(chrT == chrR);

	// 読み込み
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

				line2JLine(cell, 1, 0, 1, 1); // HACK: 改行禁止

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

		errorCase(getCount(row)); // ? chrR で終わっていない。
		errorCase(getSize(buff)); // ? chrR で終わっていない。

		releaseAutoList(row);
		releaseAutoBlock(buff);
	}

	// check
	{
		if (getCount(table)) // ? 行が１つ以上ある。
		{
			uint colcnt = getCount(getList(table, 0));
			autoList_t * row;
			uint rowidx;
			char *cell;
			uint colidx;

			errorCase(colcnt < 1); // ? 列が１つも無い。

			foreach (table, row, rowidx)
			{
				errorCase(getCount(row) != colcnt); // ? 列数が一定ではない。

				foreach (row, cell, colidx)
				{
					errorCase(cell == NULL); // ? cell == NULL
				}
			}
		}
	}
	return table;
}
