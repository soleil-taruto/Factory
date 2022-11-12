#include "C:\Factory\Common\all.h"
#include "libs\TableAccessor.h"
#include "libs\TableReader.h"
#include "libs\RedirSqlTableReader.h"

static void BackupTable(char *dir, char *tblnm)
{
	char *file = combine_cx(dir, addExt(strx(tblnm), "csv"));

	cout("< [%s]\n", tblnm);
	cout("> %s\n", file);

	TA_GetTableData(tblnm, file);

	memFree(file);
}
int main(int argc, char **argv)
{
	uint rowcntPerExec = 0;
	uint rdszPerExec = 0;
	uint batchMode = 0;

readArgs:
	if (argIs("/U"))
	{
		TA_User = nextArg();
		goto readArgs;
	}
	if (argIs("/P"))
	{
		TA_Pass = nextArg();
		goto readArgs;
	}
	if (argIs("/DB"))
	{
		TA_DBName = nextArg();
		goto readArgs;
	}
	if (argIs("/RPE"))
	{
		rowcntPerExec = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/SPE"))
	{
		rdszPerExec = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/BATCH"))
	{
		batchMode = 1;
		goto readArgs;
	}

	if (argIs("/R"))
	{
		TA_GetTableData(getArg(0), getArg(1));
		return;
	}
	if (argIs("/S"))
	{
		TA_GetTableSchema(getArg(0), getArg(1));
		return;
	}
	if (argIs("/T"))
	{
		TA_GetTableList(getArg(0));
		return;
	}
	if (argIs("/INS"))
	{
		TA_InsertCSVFile(getArg(0), NULL, getArg(1), rowcntPerExec, rdszPerExec);
		return;
	}
	if (argIs("/DEL"))
	{
		TA_DeleteCSVFile(getArg(0), NULL, getArg(1), rowcntPerExec, rdszPerExec);
		return;
	}
	if (argIs("/DELALL"))
	{
		TA_DeleteAll(nextArg());
		return;
	}
	if (argIs("/F"))
	{
		char *midFile = makeTempPath("mid");
		char *tblnm;
		char *outFile;
		autoList_t *colnms = newList();

		tblnm = nextArg();
		outFile = nextArg();

		while (hasArgs(1))
			addElement(colnms, (uint)nextArg());

		TA_GetTableData(tblnm, midFile);
		TA_TableDataFltr_cxc(midFile, colnms, outFile);

		releaseAutoList(colnms);
		removeFile(midFile);
		memFree(midFile);
		return;
	}

	if (argIs("/BACKUP"))
	{
		char *dir = nextArg();

		errorCase(!existDir(dir));

		if (hasArgs(1))
		{
			while (hasArgs(1))
			{
				BackupTable(dir, nextArg());
			}
		}
		else
		{
			char *midFile = makeTempPath("mid");
			char *nmsFile = makeTempPath("nms");
			autoList_t *tblnms;
			char *tblnm;
			uint tblnmidx;

			TA_GetTableList(midFile);
			TR_SelectWhereAnd_cxxxc(midFile, tokenize("$", '\1'), tokenize("", '\1'), tokenize("NAME", '\1'), nmsFile);
//			TA_TableDataFltr_cxc(midFile, tokenize("NAME", '\1'), nmsFile); // CSV出力だけど1列のみで改行・カンマ・引用符を含まないだろうから大丈夫だったと思う。
			tblnms = readLines(nmsFile);
			rapidSortLines(tblnms);

			if (!batchMode)
				tblnms = selectLines_x(tblnms);

			foreach (tblnms, tblnm, tblnmidx)
			{
				cmdTitle_x(xcout("ta - BACKUP %u / %u", tblnmidx, getCount(tblnms)));
				BackupTable(dir, tblnm);
			}
			cmdTitle("ta");
			releaseDim(tblnms, 1);

			removeFile(midFile);
			removeFile(nmsFile);
			memFree(midFile);
			memFree(nmsFile);
		}
		return;
	}
	if (argIs("/RESTORE"))
	{
		autoList_t *files = lsFiles(nextArg());
		char *file;
		uint index;

		foreach (files, file, index)
		{
			cmdTitle_x(xcout("ta - RESTORE %u / %u", index, getCount(files)));

			if (!_stricmp("CSV", getExt(file)))
			{
				char *tblnm = changeExt(getLocal(file), "");

				cout("< %s\n", file);
				cout("> [%s]\n", tblnm);

				TA_DeleteAll(tblnm);
				TA_InsertCSVFile(tblnm, NULL, file, rowcntPerExec, rdszPerExec);

				memFree(tblnm);
			}
		}
		cmdTitle("ta");
		releaseDim(files, 1);
		return;
	}

	if (argIs("/SCHEMA"))
	{
		char *midFile = makeTempPath("mid");
		char *nmsFile = makeTempPath("nms");
		char *schFile = makeTempPath("sch");
		char *outFile = nextArg();
		autoList_t *tblnms;
		char *tblnm;
		uint tblnmidx;
		uint colcnt;
		uint colidx;
		autoList_t *lines;
		char *line;
		FILE *fp;
		FILE *outfp;

		outfp = fileOpen(outFile, "wt");

		TA_GetTableList(midFile);
		TR_SelectWhereAnd_cxxxc(midFile, tokenize("$", '\1'), tokenize("", '\1'), tokenize("NAME", '\1'), nmsFile);
		tblnms = readLines(nmsFile);
		rapidSortLines(tblnms);

		createFile(schFile); // 2bs?

		foreach (tblnms, tblnm, tblnmidx)
		{
			cmdTitle_x(xcout("ta - SCHEMA %u / %u", tblnmidx, getCount(tblnms)));

			TA_GetTableSchema(tblnm, midFile);
			colcnt = (uint)TR_SelectWhereAnd_cxxxc(midFile, tokenize("$", '\1'), tokenize("", '\1'), tokenize("TYPE_NAME LENGTH COLUMN_NAME", ' '), schFile);
			fp = fileOpen(schFile, "rt");
			lines = newList();

			for (colidx = 0; colidx < colcnt; colidx++)
			{
				char *typnm;
				char *colln;
				char *colnm;

				typnm = neReadLine(fp);
				colln = neReadLine(fp);
				colnm = neReadLine(fp);

				addElement(lines, (uint)xcout("%s(%s)\t%s", typnm, colln, colnm));

				memFree(typnm);
				memFree(colln);
				memFree(colnm);
			}
			fileClose(fp);
			autoIndent(lines);

			writeLine(outfp, tblnm);

			foreach (lines, line, colidx)
				writeLine_x(outfp, xcout("\t%s", line));

			writeChar(outfp, '\n');

			releaseDim(lines, 1);
		}
		releaseDim(tblnms, 1);

		removeFile(midFile);
		removeFile(nmsFile);
		removeFile(schFile);
		memFree(midFile);
		memFree(nmsFile);
		memFree(schFile);

		fileClose(outfp);
		return;
	}

	error(); // 不明なコマンド引数
}
