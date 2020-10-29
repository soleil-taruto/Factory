/*
	> lss .png*
	> lssImg

	> lss . .bmp* .jpg* .jpeg* .png* .gif*
	> lssImg
*/

#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	autoList_t *imgFiles = readLines(FOUNDLISTFILE);
	char *imgFile;
	uint index;
	int mode = 0;
	char *outFile = c_getOutFile("images.html");
	FILE *fp;

	if(argIs("/F"))
	{
		mode ='F';
	}

	fp = fileOpen(outFile, "wt");
	writeLine(fp, "<html>");
	writeLine(fp, "<body>");

	foreach(imgFiles, imgFile, index)
	{
		switch(mode)
		{
		case 'F':
			writeLine_x(fp, xcout("%s<br/>", imgFile));
			writeLine_x(fp, xcout("<img src=\"%s\"/><br/>", imgFile));
			writeLine(fp, "<br/>");
			break;

		default:
			writeLine_x(fp, xcout("<img src=\"%s\"/>", imgFile));
		}
	}
	releaseDim(imgFiles, 1);

	writeLine(fp, "</body>");
	writeLine(fp, "</html>");
	fileClose(fp);

	openOutDir();
}
