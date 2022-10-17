/*
	acp.exe 入力ファイル 出力ファイル

		入力ファイル, 出力ファイル ... パス内の *P をカレントDIRのローカル名(プロジェクト名)に置き換える。

		ルートDIRから実行するとプロジェクト名を得られないためエラーになる。
*/

#include "C:\Factory\Common\all.h"

#define REP_PTN_PROJECT_NAME "*P"

int main(int argc, char **argv)
{
	char *rFile;
	char *wFile;
	char *projectName = getLocal(getCwd()); // g

	errorCase(m_isEmpty(projectName));

	LOGPOS();
	cout("P %s\n", projectName);

	rFile = makeFullPath(nextArg());
	wFile = makeFullPath(nextArg());

	LOGPOS();

	rFile = replaceLine(rFile, REP_PTN_PROJECT_NAME, projectName, 0);
	wFile = replaceLine(wFile, REP_PTN_PROJECT_NAME, projectName, 0);

	LOGPOS();
	cout("< %s\n", rFile);
	cout("> %s\n", wFile);

	copyFile(rFile, wFile);

	LOGPOS();

	memFree(rFile);
	memFree(wFile);

	LOGPOS();
}
