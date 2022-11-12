#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#define N 3
#define EXP_MAX 1000
#define BLOCKSZ 10
#define VALSZ 1000000000

typedef struct B_st {
	int Vals[BLOCKSZ];
	struct B_st *Prev;
	struct B_st *Next;
}
B_t;

static B_t Top;
static B_t *Last;

static void MulN(int n) {
	B_t *b = &Top;
	B_t *bn;
	int index;
	__int64 v = 0;

	for (; ; ) {
		for (index = 0; index < BLOCKSZ; index++) {
			v += (__int64)b->Vals[index] * n;
			b->Vals[index] = (int)(v % VALSZ);
			v /= VALSZ;
		}
		bn = b->Next;

		if (!bn) {
			if (!v) {
				break;
			}
			bn = (B_t *)calloc(1, sizeof(B_t));

			if (!bn)
				exit(1);

			b->Next = bn;
			bn->Prev = b;
			Last = bn;
		}
		b = bn;
	}
}

static void DoPrint(int n, int exp) {
	B_t *b = Last;
	int index;

	printf("%d^%d=", n, exp);

	for (index = BLOCKSZ - 1; !b->Vals[index]; index--);

	printf("%d", b->Vals[index]);

	while (0 <= --index) {
		printf("%09d", b->Vals[index]);
	}
	while (b = b->Prev) {
		for (index = BLOCKSZ - 1; 0 <= index; index--) {
			printf("%09d", b->Vals[index]);
		}
	}
	printf("\n");
}

int main(void) {
	int exp;

	Top.Vals[0] = 1;
	Last = &Top;

	DoPrint(N, 0);

	for (exp = 1; exp <= EXP_MAX; exp++) {
		MulN(N);
		DoPrint(N, exp);
	}
}
