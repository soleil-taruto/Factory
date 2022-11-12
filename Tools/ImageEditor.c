/*
	ImageEditor.exe [‰æ‘œƒtƒ@ƒCƒ‹]
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\SubTools\libs\Image.h"

static char *ImgFile;

static void CommandPrint(char *jbuffer)
{
	cout("\r%79s\rIE> %s", "", jbuffer);
}
static void ImageEditor(void)
{
	errorCase(!ImgFile); // 2bs

	SetImageId(1);
	SetImageSize(30, 30);
	SetImageId(0);
	LoadImageFile(ImgFile);

	for (; ; )
	{
		char *command = coInputLinePrn(CommandPrint);

		if (!_stricmp(command, "q"))
		{
			memFree(command);
			break;
		}
		if (!*command)
		{
			// noop
		}
		else if (!_stricmp(command, "s!"))
		{
			cout("Save: %s\n", ImgFile);
			SaveImageFile(ImgFile);
		}
		else if (!_stricmp(command, "s"))
		{
			char *file = toCreatableTildaPath(strx(ImgFile), IMAX);

			cout("Save: %s\n", file);
			SaveImageFile(file);
			memFree(file);
		}

		// TODO

		else
		{
			cout("Unknown command !\n");
		}
		memFree(command);
	}
}
int main(int argc, char **argv)
{
	isJChar(0); // preloading

	if (hasArgs(1))
		ImgFile = nextArg();
	else
		ImgFile = c_dropFile();

	ImageEditor();
}
