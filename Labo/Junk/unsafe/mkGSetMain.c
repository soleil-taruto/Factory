/*
	mkGSetMain.exe ��[�p�X ���ԃp�X�̃t�H�[�}�b�g �I�[�p�X �o�̓t�@�C���t�H�[�}�b�g�̒u�������p�^�[�� �o�̓t�@�C���t�H�[�}�b�g �o�̓t�@�C��
*/

#include "C:\Factory\Common\all.h"

static int IsNeedUpdateOneLineFile(char *file, char *lineNew)
{
	char *line = readFirstLine(file);
	int ret;

	ret = strcmp(line, lineNew);

	memFree(line);
	return ret;
}
int main(int argc, char **argv)
{
	char *leadDir;
	char *midDirFormat;
	char *trailPath;
	char *wffRepPtn;
	char *wFileFormat;
	char *wFile;

	leadDir      = nextArg();
	midDirFormat = nextArg();
	trailPath    = nextArg();
	wffRepPtn    = nextArg();
	wFileFormat  = nextArg();
	wFile        = nextArg();

	errorCase(m_isEmpty(leadDir));
	errorCase(m_isEmpty(midDirFormat));
	errorCase(m_isEmpty(trailPath));
	errorCase(m_isEmpty(wffRepPtn));
	errorCase(m_isEmpty(wFileFormat));
	errorCase(m_isEmpty(wFile));

	LOGPOS();

	errorCase(!existDir(leadDir));

	{
		autoList_t *midDirs = lsDirs(leadDir);
		char *midDir;
		uint index;
		char *foundPath = NULL;
		char *outLine;

		eraseParents(midDirs);

		// DESC
		{
			rapidSortJLinesICase(midDirs);
			reverseElements(midDirs);
		}

		foreach (midDirs, midDir, index)
		{
			if (lineExp(midDirFormat, midDir))
			{
				char *path = xcout("%s\\%s\\%s", leadDir, midDir, trailPath);

				if (accessible(path))
				{
					foundPath = path;
					break;
				}
				memFree(path);
			}
		}
		releaseDim(midDirs, 1);

		errorCase(!foundPath);

		outLine = strx(wFileFormat);
		outLine = replaceLine(outLine, wffRepPtn, foundPath, 0);

		if (!existFile(wFile) || IsNeedUpdateOneLineFile(wFile, outLine))
		{
			LOGPOS();
			writeOneLine(wFile, outLine);
		}
		memFree(foundPath);
		memFree(outLine);
	}

	LOGPOS();
}
