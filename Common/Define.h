// ��� --->
#include <conio.h>
#include <ctype.h>
#include <direct.h>
#include <dos.h>
#include <fcntl.h>
#include <io.h>
#include <limits.h>
#include <malloc.h>
#include <math.h>
#include <mbstring.h>
#include <process.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys\types.h> // sys/stat.h ����ł��邱�ƁB
#include <sys\stat.h>
#include <time.h>
// <--- ���

#include <windows.h>

#define SINT16MIN -0x7fff // SHRT_MIN + 1
#define SINT16MAX 0x7fff
#define UINT16MAX 0xffff
#define SINTMIN -0x7fffffff // INT_MIN + 1
#define SINTMAX 0x7fffffff
#define UINTMAX 0xffffffff
#define SINT64MIN -0x7fffffffffffffffi64 // _I64_MIN + 1
#define SINT64MAX 0x7fffffffffffffffui64
#define UINT64MAX 0xffffffffffffffffui64

typedef signed __int8 schar;
typedef unsigned __int8 uchar;
typedef signed __int16 sint16;
typedef unsigned __int16 uint16;
typedef signed __int32 sint;
typedef unsigned __int32 uint;
typedef signed __int64 sint64;
typedef unsigned __int64 uint64;

#define lengthof(list) \
	(sizeof(list) / sizeof(*list))

#define foreach(list, element, index) \
	for (index = 0; index < getCount(list) ? (*(uint *)&element = getElement(list, index), 1) : (*(uint *)&element = 0); index++)

#define LOGPOS2(mode) \
	cout("%s (%d) %s %u %s\n", __FILE__, __LINE__, __FUNCTION__, GetCurrentThreadId(), LOGPOS_Time(mode))

#define LOGPOS() \
	LOGPOS2(0)

#define LOGPOS_T() \
	LOGPOS2('T')

#define m_swap(lop1, lop2, TYPE_T) \
	do { \
	TYPE_T tmp = (TYPE_T)(lop1); \
	*(TYPE_T *)&(lop1) = (TYPE_T)(lop2); \
	*(TYPE_T *)&(lop2) = tmp; \
	} while (0)

// ����͕����Ȃ��B��������� �� d2i()
#define m_d2i(value) \
	(uint)((value) + 0.5)

#define m_sign(value) \
	((value) < 0 ? -1 : (0 < (value) ? 1 : 0))

#define m_isRange(value, minval, maxval) \
	((minval) <= (value) && (value) <= (maxval))

#define FOUNDLISTFILE   "C:\\Factory\\tmp\\Files.txt"
#define SECTIONLISTFILE "C:\\Factory\\tmp\\Sections.txt"

/*
	�����̏���Ƃ��ĉ��ƂȂ����߂��l
	�E10��
	�E10��
	�E9���̍ő�l+1
	�E2�{���Ă� INT_MAX ��菬����
*/
#define IMAX 1000000000

/*
	64�r�b�g�����̏���Ƃ��ĉ��ƂȂ����߂��l
	�EIMAX ^ 2
	�E100��
	�E19��
	�E18���̍ő�l+1
	�E9�{���Ă� _I64_MAX ��菬����
*/
#define IMAX_64 1000000000000000000i64

#define m_01(flag) \
	(!(flag) ? 0 : 1)

#define m_10(flag) \
	(!(flag) ? 1 : 0)

#define m_noz(a, b) \
	((a) ? (a) : (b))

typedef struct i2D_st
{
	int X;
	int Y;
}
i2D_t;

typedef struct d2D_st
{
	double X;
	double Y;
}
d2D_t;

#define zeroclear(var) \
	(memset(&(var), 0x00, sizeof((var))))
