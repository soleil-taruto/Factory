#include "C:\Factory\Common\all.h"

#define PI 3.141592653589793
#define ANGLEMAX 1570
#define RETSCALE 1000

int main(int argc, char **argv)
{
	autoList_t *lines = newList();
	uint angle;

	for (angle = 0; angle <= ANGLEMAX; angle++)
	{
		addElement(lines, (uint)xcout("%u", m_d2i(RETSCALE * sin(angle * PI / 2 / ANGLEMAX))));
	}
	writeLines_cx(c_getOutFile("Sine90_B.txt"), lines);
	openOutDir();
}
