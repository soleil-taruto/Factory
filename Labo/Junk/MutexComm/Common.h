#pragma once

#include "C:\Factory\Common\all.h"

enum
{
	M_MUTEX_0,
	M_MUTEX_1,
	M_MUTEX_2,
	M_BIT_0_0,
	M_BIT_0_1,
	M_BIT_0_2,
	M_BIT_1_0,
	M_BIT_1_1,
	M_BIT_1_2,

	M_NUM, // num of M_
};

void M_Init(void);
void M_Fnlz(void);
void M_Set(uint index, uint status);
uint M_Get(uint index);
