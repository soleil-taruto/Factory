#include "C:\Factory\Common\all.h"

#define PI 3.141592653589793

static double GetAngle(double x, double y)
{
	double r1 = 0.0;
	double r2 = 0.5 * PI;
	double rm;
	double t;
	uint count;

	if (y < 0.0) return 2.0 * PI - GetAngle(x, -y);
	if (x < 0.0) return PI - GetAngle(-x, y);

	if (x == 0.0) return PI / 2.0;
	if (y == 0.0) return 0.0;

	t = y / x;

	for (count = 10; ; count--)
	{
		rm = (r1 + r2) / 2.0;

		if (!count)
			break;

		if (t < tan(rm))
			r2 = rm;
		else
			r1 = rm;
	}
	return rm;
}
int main(int argc, char **argv)
{
	double r;

	for (r = -0.1; r <= 2.0 * PI + 0.1; r += 0.0001)
	{
		double x = cos(r);
		double y = sin(r);
		double rr;

		rr = GetAngle(x, y);

		cout("%f %f (%f) [%f, %f]\n", r, rr, fabs(r - rr), x, y);
	}
}
