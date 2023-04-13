/*
	éÂÇ… C:\app\Kit\* Ç∆ÇÃòAågÇëzíËÇ∑ÇÈÅBä÷òA -> innerResPathFltr

	C:\app\Kit îpé~ @ 2023.4.13
*/

#include "Collabo.h"

static char *S_GetCollaboPath(char *innerPath, int (*existFunc)(char *))
{
	char *path;

	errorCase(m_isEmpty(innerPath));

	if (isAbsPath(innerPath))
	{
		if (isFactoryDirDisabled())
		{
			path = combine(getSelfDir(), getLocal(innerPath));

			errorCase(!_stricmp(path, getSelfFile()));

			if (existFunc(path))
				goto foundPath;

			memFree(path);
		}
		path = makeFullPath(innerPath);

		if (existFunc(path))
			goto foundPath;
	}
	else
	{
		path = combine(getSelfDir(), innerPath);

		if (existFunc(path))
			goto foundPath;

		if (isFactoryDirDisabled())
		{
			memFree(path);
			path = combine(getSelfDir(), getLocal(innerPath));

			errorCase(!_stricmp(path, getSelfFile()));

			if (existFunc(path))
				goto foundPath;
		}
	}
	memFree(path);
	error_m(xcout("[%s]Ç™å©Ç¬Ç©ÇËÇ‹ÇπÇÒÅB", innerPath));

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
