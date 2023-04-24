/*
	CheckGitRelease [/D 直接チェックDIR | ルートDIR]
*/

#include "C:\Factory\Common\all.h"

#define LOCAL_GIT_RELEASE_BAT "GitRelease.bat"
#define LOCAL_RUN_ME_FIRST_BAT "__run_me_first.bat_"
#define LOCAL_LICENSE "LICENSE"
#define LOCAL_LICENSE_IGNORE "LIGNORE" // LICENSEファイルをまだ？設定しない場合、暫定的に？これを置いておく。

static autoList_t *KnownRepos;
static autoList_t *ErrorFiles;
static int ErrorFound;

static void FoundError(char *message)
{
	cout("★ %s\n", message);
	ErrorFound = 1;
}
static char *ProjectLocalDirToRepositoryName(char *localDir)
{
	if (lineExp("<1,azAZ><8,09>_<1,,>", localDir))
	{
		localDir += 10;
	}
	return localDir;
}
static void CheckGitRelease(char *dir)
{
	dir = makeFullPath(dir);
	cout("チェック対象ディレクトリ ⇒ %s\n", dir);
	ErrorFound = 0;

	addCwd(dir);

	if (!existFile(LOCAL_GIT_RELEASE_BAT))
	{
		FoundError(LOCAL_GIT_RELEASE_BAT " が見つかりません。");
	}
	else
	{
		{
			char *text = readText_b(LOCAL_GIT_RELEASE_BAT);
			char *text2 = xcout(
				"@rem このフォルダのローカル名は Repo または x99999999_Repo (Repo=リリース先のリポジトリ名) であること。\r\n"
				"\r\n"
				"IF NOT EXIST .\\GitRelease.bat GOTO END\r\n"
				"CALL qq\r\n"
				"C:\\Factory\\SubTools\\GitFactory.exe /ow . C:\\huge\\GitHub\\%s\r\n"
				"rem CALL C:\\temp\\go.bat\r\n"
				":END\r\n"
				,ProjectLocalDirToRepositoryName(getLocal(dir))
				);

			if (strcmp(text, text2))
			{
				FoundError(LOCAL_GIT_RELEASE_BAT " の内容に問題があります。(bad contents)");
			}
			memFree(text);
			memFree(text2);
		}

		{
			char *repoDir = combine("C:\\huge\\GitHub", ProjectLocalDirToRepositoryName(getLocal(dir)));

			if (!existDir(repoDir))
			{
				FoundError(LOCAL_GIT_RELEASE_BAT " の内容に問題があります。(no repoDir)");
			}
			memFree(repoDir);
		}

		{
			char *repo = ProjectLocalDirToRepositoryName(getLocal(dir));

			if (findJLineICase(KnownRepos, repo) < getCount(KnownRepos))
			{
				FoundError(LOCAL_GIT_RELEASE_BAT " の内容に問題があります。(リポジトリの重複)");
			}
			else
			{
				addElement(KnownRepos, (uint)strx(repo));
			}
		}
	}

	if (!existFile(LOCAL_RUN_ME_FIRST_BAT))
	{
		FoundError(LOCAL_RUN_ME_FIRST_BAT " が見つかりません。");
	}
	else
	{
		char *text = readText_b(LOCAL_RUN_ME_FIRST_BAT);
		char *text2 = strx(
			"C:\\Factory\\bat\\devrunmefirst.bat\r\n"
			);

		if (strcmp(text, text2))
		{
			FoundError(LOCAL_RUN_ME_FIRST_BAT " の内容に問題があります。");
		}
		memFree(text);
		memFree(text2);
	}

	if (m_01(existFile(LOCAL_LICENSE)) + m_01(existFile(LOCAL_LICENSE_IGNORE)) != 1)
	{
		FoundError("ライセンスファイル (LICENSE) が見つかりません。");
	}
	if (existFile(LOCAL_LICENSE) && getFileSize(LOCAL_LICENSE) == 0)
	{
		FoundError("ライセンスファイル (LICENSE) が空っぽです。");
	}
	if (existFile(LOCAL_LICENSE_IGNORE) && getFileSize(LOCAL_LICENSE_IGNORE) != 0)
	{
		FoundError(LOCAL_LICENSE_IGNORE " が空ではありません。");
	}

	unaddCwd();

	if (ErrorFound)
		addElement(ErrorFiles, (uint)combine(dir, LOCAL_GIT_RELEASE_BAT));

	memFree(dir);
}

static void CheckDir(char *dir);

static void FindProjectDir(char *rootDir)
{
	autoList_t *dirs = lsDirs(rootDir);
	char *dir;
	uint index;

	foreach (dirs, dir, index)
	{
		CheckDir(dir);
	}
	releaseDim(dirs, 1);
}
static void CheckDir(char *dir)
{
	char *gitReleaseBat = combine(dir, LOCAL_GIT_RELEASE_BAT);

	if (existFile(gitReleaseBat))
	{
		CheckGitRelease(dir);
	}
	else
	{
		FindProjectDir(dir);
	}
	memFree(gitReleaseBat);
}
int main(int argc, char **argv)
{
	KnownRepos = newList();
	ErrorFiles = newList();

	errorCase_m(argIs("/C"), "廃止オプション"); // zantei

	if (argIs("/D"))
	{
		CheckGitRelease(nextArg());
	}
	else if (hasArgs(1))
	{
		CheckDir(nextArg());
	}
	else
	{
		CheckDir("C:\\Dev");
	}

	cout("\n");

	if (getCount(ErrorFiles))
	{
		cout("★★★ [ %u ] 件のエラーが見つかりました。\n", getCount(ErrorFiles));
	}
	else
	{
		cout("エラーは見つかりませんでした。\n");
	}
	writeLines(FOUNDLISTFILE, ErrorFiles);
}
