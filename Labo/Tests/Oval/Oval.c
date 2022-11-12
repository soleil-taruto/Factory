#include "C:\Factory\Common\all.h"
#include "C:\Factory\SubTools\libs\bmptbl.h"

static autoTable_t *Oval;

static void MakeOval(void)
{
	int x;
	int y;

	Oval = newTable(getZero, noop_u);

	resizeTable(Oval, 1000, 1000);

	for (x = 0; x < 1000; x++)
	for (y = 0; y < 1000; y++)
	{
		double dx = x - 500.0;
		double dy = y - 500.0;
		double r;
		double d;
		int lv;

		dy *= 4.0 / 3.0; // ’ZŒa 300.0

		r = sqrt(dx * dx + dy * dy);

		d = r - 400.0;
		d = fabs(d);

		lv = (int)(d * 50.0 + 0.5);
		m_minim(lv, 255);
		lv *= 0x010101;

		setTableCell(Oval, x, y, lv);
	}
}
int main(int argc, char **argv)
{
	MakeOval();
	tWriteBMPFile(c_getOutFile("Oval.bmp"), Oval);
	openOutDir();
}
