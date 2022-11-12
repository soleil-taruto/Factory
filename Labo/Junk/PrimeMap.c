#include <stdio.h>

//	#define ODPS_SIZE 100000
//	#define ODPS_SIZE 300000
	#define ODPS_SIZE 1000000
//	#define ODPS_SIZE 3000000
//	#define ODPS_SIZE 10000000
//	#define ODPS_SIZE 30000000

#define PRIME_MAX ((ODPS_SIZE) * 64 - 1)

typedef unsigned int uint;

static uint OdPs[ODPS_SIZE];

#define IsNOP(n) \
	(OdPs[(n) / 64] & ((uint)1 << (((n) / 2) % 32)))

#define PutNOP(n) \
	(OdPs[(n) / 64] |= (uint)1 << (((n) / 2) % 32))

static void MkOdPs(void)
{
	int n;
	int c;

	OdPs[0] = 1;

	for (n = 3; n * n <= PRIME_MAX; n += 2)
		if (!IsNOP(n))
			for (c = n * n; c <= PRIME_MAX; c += n * 2)
				PutNOP(c);
}
static int IsPrime(int n)
{
	if (n < 2)
		return 0;

	if (n == 2)
		return 1;

	if (n % 2 == 0)
		return 0;

	if (PRIME_MAX < n)
	{
		printf("Error: PRIME_MAX < n\n");
		return 0;
	}
	return !IsNOP(n);
}
int main()
{
	int n;
	int c = 0;

	MkOdPs();

	printf("MkOdPs done!\n");

	for (n = 1; n <= PRIME_MAX; n++)
		if (IsPrime(n))
			c++;

	printf("1 - %d : %d\n", PRIME_MAX, c);
}
