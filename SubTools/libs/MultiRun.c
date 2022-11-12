#include "MultiRun.h"

/*
	getCount(progFiles) == 0 でもok
*/
void MultiRun_Programs_Mode(autoList_t *progFiles, uint mode)
{
	autoList_t *args = newList();
	char *argsFile = makeTempPath(NULL);
	char *progFile;
	uint index;

	LOGPOS();

	errorCase(!progFiles);
	errorCase(!m_isRange(mode, 0, 2));

	errorCase(!existFile(FILE_TOOLKIT_EXE)); // 外部コマンド存在確認

	addElement(args, (uint)strx("/MULTI-RUN"));
	addElement(args, (uint)xcout("%u", mode));

	foreach (progFiles, progFile, index)
	{
		errorCase(!isLine(progFile));

		addElement(args, (uint)strx(progFile));
	}
	writeLines(argsFile, args);
	releaseDim(args, 1);

	coExecute_x(xcout(FILE_TOOLKIT_EXE " //R \"%s\"", argsFile));

	removeFile(argsFile);
	memFree(argsFile);

	LOGPOS();
}
void MultiRun_Programs(autoList_t *progFiles)
{
	MultiRun_Programs_Mode(progFiles, 1);
}
void MultiRun_Commands_Mode(autoList_t *commandTable, uint mode)
{
	autoList_t *batFiles = newList();
	autoList_t *commands;
	uint index;
	char *jStamp = makeJStamp(NULL, 1);

	errorCase(!commandTable);

	foreach (commandTable, commands, index)
	{
		char *batFile = makeTempPath("bat");

		errorCase(!commands);

		{
			char *line;
			uint line_index;

			foreach (commands, line, line_index)
				errorCase(!line); // 空行はok
//				errorCase(m_isEmpty(line));
		}

		{
			FILE *fp = fileOpen(batFile, "wt");

			writeLine_x(fp, xcout("rem MULTI_RUN -- %s [%u]", jStamp, index));
			writeLines2Stream(fp, commands);

			fileClose(fp);
		}

		addElement(batFiles, (uint)batFile);
	}
	MultiRun_Programs_Mode(batFiles, mode);

	{
		char *batFile;

		foreach (batFiles, batFile, index)
			removeFile(batFile);
	}

	releaseDim(batFiles, 1);
	memFree(jStamp);
}
void MultiRun_Commands(autoList_t *commandTable)
{
	MultiRun_Commands_Mode(commandTable, 1);
}
