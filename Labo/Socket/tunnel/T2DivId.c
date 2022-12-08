/*
	Tunnel2.exe の出力ファイルを、コネクション毎にファイルに分ける。

	- - -

	T2DivId.exe [入力ファイル]
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\rbTree.h"

static void T2DivId(char *rFile)
{
	rbTree_t *id2OutFile = rbCreateTree(echo_u, simpleComp, noop_u);
	FILE *rfp = fileOpen(rFile, "rt");

	for (; ; )
	{
		char *line = readLine(rfp);
		char *p;
		uint64 stamp;
		uint id;
		int kind;
		char *wFile;

		if (!line)
			break;

		errorCase(!lineExp("<17,09>,<4,10,09>,<1,CDRS>,<>", line));

		p = line + 17;
		p[0] = '\0';
		stamp = toValue64(line);
		p[0] = ',';

		p = strchr(line + 22, ',');
		p[0] = '\0';
		id = toValue(line + 18);
		p[0] = ',';

		kind = p[1];

		if (!rbtHasKey(id2OutFile, id))
		{
			wFile = getOutFile_x(xcout("%I64u.csv", stamp));
			errorCase(existFile(wFile));
			rbtAddValue(id2OutFile, id, (uint)wFile);
		}
		else
		{
			wFile = (char *)rbtGetLastAccessValue(id2OutFile);
		}
		addLine2File(wFile, line);

		if (kind == 'D')
		{
			char *endWFile = changeExt(wFile, "");

			endWFile = addLine_x(endWFile, xcout("-%I64u-%u.csv", stamp, id));
			moveFile(wFile, endWFile);
			memFree(endWFile);

			rbtUnaddLastAccessValue(id2OutFile);
			memFree(wFile);
		}
		memFree(line);
	}
	fileClose(rfp);

	rbtCallAllValue(id2OutFile, (void (*)(uint))memFree);
	rbReleaseTree(id2OutFile);

	openOutDir();
}
int main(int argc, char **argv)
{
	T2DivId(hasArgs(1) ? nextArg() : c_dropFile());
}
