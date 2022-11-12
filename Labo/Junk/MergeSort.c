/*
	1.txt -> 2.txt
		MD tmp
		MregeSort.exe /DDS 1.txt tmp 100000000
		MergeSort.exe /S tmp 2.txt
		RD /S /Q tmp

		/DDS の最後の引数がパートのバイト数の目安になる。
		空きメモリの半分くらいが安全か？
		パート数はファイルディスクリプタの上限よりある程度は少ないこと。
*/

#include "C:\Factory\Common\all.h"

typedef struct Stream_st
{
	FILE *FStrm;
	struct Stream_st *Strm1;
	struct Stream_st *Strm2;
	char *BackedLine;
}
Stream_t;

static void ReleaseStream(Stream_t *i)
{
	if (i->FStrm)
		fileClose(i->FStrm);

	if (i->Strm1)
		ReleaseStream(i->Strm1);

	if (i->Strm2)
		ReleaseStream(i->Strm2);

	if (i->BackedLine)
		memFree(i->BackedLine);

	memFree(i);
}
static char *CMS_ReadLine(Stream_t *i)
{
	char *retLine;

	if (i->BackedLine)
	{
		retLine = i->BackedLine;
		i->BackedLine = NULL;
	}
	else if (i->FStrm)
	{
		retLine = readLine(i->FStrm);
	}
	else
	{
		char *line1 = CMS_ReadLine(i->Strm1);
		char *line2 = CMS_ReadLine(i->Strm2);

		if (!line1)
		{
			retLine = line2;
		}
		else if (!line2)
		{
			retLine = line1;
		}
		else if (0 < strcmp(line1, line2)) // ? 1 > 2
		{
			retLine = line2;
			i->Strm1->BackedLine = line1;
		}
		else // ? 1 < 2 or 1 == 2
		{
			retLine = line1;
			i->Strm2->BackedLine = line2;
		}
	}
	return retLine;
}
static void CascMergeSort(autoList_t *divfiles, char *outputfile) // Cascade? Merge Sort
{
	Stream_t *root;

	errorCase(getCount(divfiles) < 1);

	{
		autoList_t *strms = newList();
		char *file;
		uint index;

		foreach (divfiles, file, index)
		{
			Stream_t *i = (Stream_t *)memCalloc(sizeof(Stream_t));

			i->FStrm = fileOpen(file, "rt");

			addElement(strms, (uint)i);
		}
		while (1 < getCount(strms))
		{
			Stream_t *i = (Stream_t *)memCalloc(sizeof(Stream_t));

			i->Strm1 = (Stream_t *)desertElement(strms, 0);
			i->Strm2 = (Stream_t *)desertElement(strms, 0);

			addElement(strms, (uint)i);
		}
		root = (Stream_t *)getElement(strms, 0);
		releaseAutoList(strms);
	}

	{
		FILE *fp = fileOpen(outputfile, "wt");
		char *line;
		uint64 linecnt;

		for (linecnt = 0; ; linecnt++)
		{
			if (eqIntPulseSec(2, NULL))
				cout("linecnt: %I64u\n", linecnt);

			line = CMS_ReadLine(root);

			if (!line)
				break;

			writeLine_x(fp, line);
		}
		fileClose(fp);
		cout("linecnt: %I64u done\n", linecnt);
	}

	ReleaseStream(root);
}
int main(int argc, char **argv)
{
	if (argIs("/D"))
	{
		char *inputfile;
		char *divdir;
		char *divfile;
		uint linenummax;
		FILE *rfp;
		FILE *wfp;
		uint divcnt;
		uint linecnt;
		char *line;

		inputfile = nextArg();
		divdir = nextArg();
		linenummax = toValue(nextArg());

		errorCase(!existFile(inputfile));
		errorCase(!existDir(divdir));
		errorCase(!linenummax);

		rfp = fileOpen(inputfile, "rt");

		for (divcnt = 0; ; divcnt++)
		{
			divfile = combine_cx(divdir, xcout("%010u.div", divcnt));
			cout("divfile: %s\n", divfile);
			wfp = fileOpen(divfile, "wt");

			for (linecnt = 0; linecnt < linenummax; linecnt++)
			{
				line = readLine(rfp);

				if (!line)
					break;

				writeLine_x(wfp, line);
			}
			fileClose(wfp);

			if (linecnt < linenummax)
			{
				if (!linecnt)
					removeFile(divfile);

				memFree(divfile);
				break;
			}
			memFree(divfile);
		}
		fileClose(rfp);
		cout("done\n");
		return;
	}
	if (argIs("/DS"))
	{
		char *divdir;
		autoList_t *files;
		char *file;
		uint index;
		autoList_t *lines;

		divdir = nextArg();
		files = ls(divdir);

		/*
			divdir を直接書き換えるので安全のため、全てファイルで ".div" であること前提。
		*/
		foreach (files, file, index)
			errorCase(_stricmp("div", getExt(file)));

		foreach (files, file, index)
		{
			cout("file: %s\n", file);

			lines = readLines(file);
			rapidSortLines(lines);
			writeLines_cx(file, lines);
		}
		releaseDim(files, 1);
		return;
	}
	if (argIs("/DDS")) // "/D" + "/DS"
	{
		char *inputfile;
		char *divdir;
		uint charnummax;
		FILE *fp;
		uint divcnt;
		uint charcnt;
		autoList_t *lines;
		char *line;
		char *divfile;

		inputfile = nextArg();
		divdir = nextArg();
		charnummax = toValue(nextArg());

		errorCase(!existFile(inputfile));
		errorCase(!existDir(divdir));
		errorCase(!charnummax);

		fp = fileOpen(inputfile, "rt");

		for (divcnt = 0; ; divcnt++)
		{
			charcnt = 0;
			lines = newList();

			while (line = readLine(fp))
			{
				charcnt += strlen(line);
				addElement(lines, (uint)line);

				if (charnummax < charcnt || charnummax / 100 < getCount(lines))
					break;
			}
			cout("charcnt: %u\n", charcnt);
			cout("linecnt: %u\n", getCount(lines));

			if (getCount(lines) == 0)
			{
				releaseAutoList(lines);
				break;
			}
			rapidSortLines(lines);

			divfile = combine_cx(divdir, xcout("%010u.div", divcnt));
			cout("divfile: %s\n", divfile);
			writeLines(divfile, lines);
			memFree(divfile);

			releaseDim(lines, 1);
		}
		fileClose(fp);
		return;
	}
	if (argIs("/S"))
	{
		char *divdir;
		char *outputfile;
		autoList_t *files;
		char *file;
		uint index;

		divdir = nextArg();
		outputfile = nextArg();
		files = ls(divdir);

		CascMergeSort(files, outputfile);

		releaseDim(files, 1);
	}
}
