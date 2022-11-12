#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	autoList_t *dirs = lsDirs("C:\\huge\\GitHub");
	char *dir;
	uint dir_index;

	foreach (dirs, dir, dir_index)
	{
		autoList_t *files = lssFiles(dir);
		char *file;
		uint file_index;

		foreach (files, file, file_index)
		{
			if (mbs_stristr(file, "\\.git"))
				continue;

			cout("Insx %s\n", file);

			{
				autoBlock_t *fileData = readBinary(file);
				autoBlock_t gab;

//				insertBytes(fileData, 0, gndBlockLineVar("x", gab)); // utf-16 Ç∆Ç© bom Ç™Ç†ÇÈÇÃÇ≈ÅAng
				addBytes(fileData, gndBlockLineVar("x", gab));

				writeBinary(file, fileData);
				releaseAutoBlock(fileData);
			}
		}
		releaseDim(files, 1);
	}
	releaseDim(dirs, 1);
}
