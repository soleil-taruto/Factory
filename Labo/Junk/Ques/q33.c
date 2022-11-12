#include "C:\Factory\Common\all.h"

typedef struct Numb_st
{
	char Caption[100];
	int Numer;
	int Denom;
}
Numb_t;

Numb_t MkNumb_x(char *caption, int numer, int denom)
{
	Numb_t i;

	strcpy(i.Caption, caption);
	i.Numer = numer;
	i.Denom = denom;

	memFree(caption);

	return i;
}

static void Analysis(Numb_t *s, int snum)
{
	int a;
	int b;
	int e;

	if (snum == 1)
	{
		if (s[0].Numer == s[0].Denom * 10) // ? == 10
		{
			cout("\t%s\n", s[0].Caption);
		}
		return;
	}

	for (e = 0; e < 4; e++)
	for (b = 0; b < snum; b++)
	for (a = 0; a < snum; a++)
	{
		if (a != b)
		{
			Numb_t *sa = s + a;
			Numb_t *sb = s + b;
			Numb_t ans;
			int n;
			int d;

			switch (e)
			{
			case 0: // +
				n = sa->Numer * sb->Denom + sb->Numer * sa->Denom;
				d = sa->Denom * sb->Denom;
				break;

			case 1: // -
				n = sa->Numer * sb->Denom - sb->Numer * sa->Denom;
				d = sa->Denom * sb->Denom;
				break;

			case 2: // *
				n = sa->Numer * sb->Numer;
				d = sa->Denom * sb->Denom;
				break;

			case 3: // /
				if (sb->Numer == 0) continue; // ? zero divide
				n = sa->Numer * sb->Denom;
				d = sa->Denom * sb->Numer;
				break;

			default:
				error();
			}
			ans = MkNumb_x(xcout("(%s %c %s)", sa->Caption, "+-*/"[e], sb->Caption), n, d);

			{
				Numb_t *t = na_(Numb_t, snum - 1);
				int si;
				int ti = 0;

				for (si = 0; si < snum; si++)
				{
					if (si != a && si != b)
					{
						t[ti++] = s[si];
					}
				}
				t[ti] = ans;
				Analysis(t, snum - 1);
				memFree(t);
			}
		}
	}
}

int main(int argc, char **argv)
{
	int a;
	int b;
	int c;
	int d;

	for (d = 1; d <= 9; d++)
	for (c = 1; c <= d; c++)
	for (b = 1; b <= c; b++)
	for (a = 1; a <= b; a++)
	{
		Numb_t s[4];

		s[0] = MkNumb_x(xcout("%d", a), a, 1);
		s[1] = MkNumb_x(xcout("%d", b), b, 1);
		s[2] = MkNumb_x(xcout("%d", c), c, 1);
		s[3] = MkNumb_x(xcout("%d", d), d, 1);

		cout("%d %d %d %d\n", a, b, c, d);

		Analysis(s, 4);
	}
}
