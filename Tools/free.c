#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	updateMemory();

	cout("メモリ使用率 = %u %%\n", lastMemoryLoad);
	cout("物理メモリ空き = %s バイト\n", c_thousandComma(xcout("%I64u", lastMemoryFree)));
	cout("物理メモリ容量 = %s バイト\n", c_thousandComma(xcout("%I64u", lastMemorySize)));
	cout("仮想メモリ空き = %s バイト\n", c_thousandComma(xcout("%I64u", lastVirtualFree)));
	cout("仮想メモリ拡張 = %s バイト\n", c_thousandComma(xcout("%I64u", lastExVirtualFree)));
	cout("仮想メモリ容量 = %s バイト\n", c_thousandComma(xcout("%I64u", lastVirtualSize)));
	cout("ページサイズ空き = %s バイト\n", c_thousandComma(xcout("%I64u", lastPageFileFree)));
	cout("ページサイズ最大 = %s バイト\n", c_thousandComma(xcout("%I64u", lastPageFileSize)));
}
