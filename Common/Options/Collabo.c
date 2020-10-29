/*
	主に C:\app\Kit\* との連携を想定する。関連 -> innerResPathFltr
*/

#include "Collabo.h"

static char *S_GetCollaboPath(char *innerPath, int (*existFunc)(char *))
{
	char *path;

	errorCase(m_isEmpty(innerPath));

	if(isAbsPath(innerPath))
	{
		if(isFactoryDirDisabled())
		{
			path = combine(getSelfDir(), getLocal(innerPath));

			errorCase(!_stricmp(path, getSelfFile()));

			if(existFunc(path))
				goto foundPath;

			memFree(path);
		}
		path = makeFullPath(innerPath);

		if(existFunc(path))
			goto foundPath;
	}
	else
	{
		path = combine(getSelfDir(), innerPath);

		if(existFunc(path))
			goto foundPath;

		if(isFactoryDirDisabled())
		{
			memFree(path);
			path = combine(getSelfDir(), getLocal(innerPath));

			errorCase(!_stricmp(path, getSelfFile()));

			if(existFunc(path))
				goto foundPath;
		}
	}
	memFree(path);
	error_m(xcout("[%s]が見つかりません。", innerPath));

foundPath:
	cout("COLLABO_PATH: %s\n", path);
	return path;
}
char *GetCollaboFile(char *innerPath)
{
	return S_GetCollaboPath(innerPath, existFile);
}
char *GetCollaboDir(char *innerPath)
{
	return S_GetCollaboPath(innerPath, existDir);
}
