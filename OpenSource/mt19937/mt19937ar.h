#pragma once

void mt19937_init_genrand(unsigned long s);
void mt19937_init_by_array(unsigned long init_key[], int key_length);
unsigned long mt19937_genrand_int32(void);
long mt19937_genrand_int31(void);
double mt19937_genrand_real1(void);
double mt19937_genrand_real2(void);
double mt19937_genrand_real3(void);
double mt19937_genrand_res53(void);
int mt19937_main(void);
