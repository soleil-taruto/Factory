#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	autoList_t *files = lssFiles(".");
	char *file;
	uint index;

	LOGPOS();

	foreach (files, file, index)
	{
		LOGPOS();

		if (
			!_stricmp("h", getExt(file)) ||
			!_stricmp("c", getExt(file)) ||
			!_stricmp("cpp", getExt(file))
			)
		{
			autoList_t *lines = readLines(file);
			char *line;
			uint line_index;
			int deleted = 0;

			LOGPOS();

			foreach (lines, line, line_index)
			{
				if (lineExp("//// #### DELETED ==== <7,09> $$$$ ////\t<>", line))
				{
					LOGPOS();
					memFree(line);
					setElement(lines, line_index, 0);
					deleted = 1;
				}
			}
			if (deleted)
			{
				LOGPOS();
				removeZero(lines);
				semiRemovePath(file);
				writeLines(file, lines);
			}
			releaseDim(lines, 1);

			LOGPOS();
		}
	}
	releaseDim(files, 1);

	LOGPOS();
}
