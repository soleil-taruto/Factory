#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\xml.h"

static void ReadWriteXml(char *rXmlFile)
{
	XNode_t *root = readXMLFile(rXmlFile);

	writeXMLFile(c_getOutFile("output.xml"), root);

	ReleaseXNode(root);
	openOutDir();
}
int main(int argc, char **argv)
{
	ReadWriteXml(nextArg());
}
