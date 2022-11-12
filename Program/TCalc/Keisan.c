/*
	C:\Factory\Tools\Keisan.c ���番�� @ 2015.7.6

	----

	12 * 34 + 56 / 78
		Keisan.exe 12 * 34 = L ; 56 / 79 = R ; @L + @R
		Keisan.exe 56 / 79 = R ; 12 * 34 + @R

	12 - 34 / (56 + 78)
		Keisan.exe 56 + 78 !/ 34 !- 12

	�Q�̕�������������100�ʂ܂ŋ��߂�B
		Keisan.exe /b 100 2 r 2

	����       �L��   ����     �E��       �]���l     ���x
	---------------------------------------------------------
	���Z       +      ���{��   ���{��     ���{��     �ۂߖ���
	���Z       -      ���{��   ���{��     ���{��     �ۂߖ���
	��Z       *      ���{��   ���{��     ���{��     �ۂߖ���
	���Z       /      ���{��   ���{��     ���{��     ������ basement �ʂ܂�, ������ basement + 1 �ʈȉ��؂�̂�
	��]       M      ���{��   ���{��     ���{��     �ۂߖ���
	�ׂ���     P      ���{��   10�iuint   ���{��     �ۂߖ���
	�ׂ���     R      ���{��   10�iuint   ���{��     ������ basement �ʂ܂�, ������ basement + 1 �ʈȉ��؂�̂�
	�ΐ�       L      ���{��   ���{��     10�iuint   ����,                   ������ 1 �ʈȉ��؂�̂�
	�i���ϊ�   X      ���{��   10�iuint   ���{��     ������ basement �ʂ܂�, ������ basement + 1 �ʈȉ��؂�̂�

	'10�iuint' �� RADIX �Ɋւ�炸 10 �i�� 0 �` 4294967295
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\BigInt.h"
#include "C:\Factory\Common\Options\Calc.h"

static char *LeftPad(char *line)
{
	if (strlen(line) & 1)
	{
		static char *padLine;

		memFree(padLine);
		padLine = xcout("0%s", line);
		line = padLine;
	}
	return line;
}
static autoList_t *MakeHexOp(char *line)
{
	autoBlock_t *bop = makeBlockHexLine(LeftPad(line));
	autoList_t *op = newList();
	uint index;
	uint value;

	reverseBytes(bop);

	for (index = 0; index < (getSize(bop) + 3) / 4; index++)
	{
		value = refByte(bop, index * 4) +
			refByte(bop, index * 4 + 1) * 0x100 +
			refByte(bop, index * 4 + 2) * 0x10000 +
			refByte(bop, index * 4 + 3) * 0x1000000;

		addElement(op, value);
	}
	releaseAutoBlock(bop);
	return op;
}
static void DispHexOp(autoList_t *op, uint bound, char *title)
{
	uint index;
	uint value;

	op = copyAutoList(op);

	if (getCount(op) < bound)
	{
		setCount(op, bound);
	}
	reverseElements(op);
	cout("%s: ", title);

	foreach (op, value, index)
	{
		if (index)
		{
			cout(":");
		}
		cout("%08x", value);
	}
	cout("\n");
	releaseAutoList(op);
}
static void HexKeisan(void)
{
	uint bound = 8;
	autoList_t *op1;
	autoList_t *op2;
	autoList_t *op3;
	autoList_t *ans;
	int op;

// readArgs:
	if (argIs("/B")) // Bound
	{
		bound = toValue(nextArg());
//		goto readArgs;
	}

	op1 = MakeHexOp(nextArg());

	while (hasArgs(2))
	{
		op = nextArg()[0];
		op2 = MakeHexOp(nextArg());

		cout("OPERATOR: %c\n", op);
		DispHexOp(op1, bound, "OP-001");
		DispHexOp(op2, bound, "OP-002");

		switch (m_tolower(op))
		{
		case '+': ans = addBigInt(op1, op2); break;
		case '-':
			ans = subBigInt(op1, op2);
			errorCase(!ans);
			break;

		case '*': ans = mulBigInt(op1, op2); break;
		case '/': ans = divBigInt(op1, op2, NULL); break;
		case 'm': ans = modBigInt(op1, op2); break;
		case 'p': ans = powerBigInt(op1, op2); break;
		case 'o':
			op3 = MakeHexOp(nextArg());
			DispHexOp(op3, bound, "OP-003");
			ans = modPowerBigInt(op1, op2, op3);
			releaseAutoList(op3);
			break;

		default:
			error();
		}
		DispHexOp(ans, bound, "ANSWER");

		releaseAutoList(op1);
		releaseAutoList(op2);

		op1 = ans;
	}
	releaseAutoList(op1);
}

static autoList_t *MemoryKeys;
static autoList_t *MemoryValues;

static void InitMemory(void)
{
	MemoryKeys = newList();
	MemoryValues = newList();
}
static void FnlzMemory(void)
{
	releaseDim(MemoryKeys, 1);
	releaseDim(MemoryValues, 1);
}
static void SetMemory(char *key, char *value)
{
	uint index = findLineCase(MemoryKeys, key, 1);

	if (index < getCount(MemoryKeys))
	{
		memFree(getLine(MemoryValues, index));
		setElement(MemoryValues, index, (uint)strx(value));
	}
	else
	{
		addElement(MemoryKeys, (uint)strx(key));
		addElement(MemoryValues, (uint)strx(value));
	}
}
static char *GetMemory(char *key)
{
	return getLine(MemoryValues, findLineCase(MemoryKeys, key, 1));
}

static char *InsSepOp(char *line, uint ranks)
{
	if (ranks) // x , n -> x �� n �����݂� ',' ������B
	{
		uint decpidx = (uint)strchrEnd(line, '.') - (uint)line;
		uint index;

		for (index = decpidx + ranks + 1; index < strlen(line); index += ranks + 1)
			line = insertChar(line, index, ',');

		for (index = decpidx; ranks < index; )
		{
			index -= ranks;
			line = insertChar(line, index, ',');
		}
		if (startsWith(line, "-,"))
			eraseChar(line + 1);
	}
	else // x , 0 -> ������Ԃ��B
	{
		char *swrk = xcout("%u", strlen(line));
		memFree(line);
		line = swrk;
	}
	return line;
}
static void Main2(void)
{
	uint radix = 10;
	uint basement = 10;
	char *op1;
	char *pop;
	char *op2;
	char *ans;

	InitMemory();

	if (argIs("/H")) // Hex
	{
		HexKeisan();
		goto endFunc;
	}

readArgs:
	if (argIs("/R") || argIs("/X")) // Radix
	{
		radix = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/B")) // Basement
	{
		basement = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/D"))
	{
		calcBracketedDecimalMin = toValue(nextArg());
		goto readArgs;
	}

	op1 = strx(nextArg());

	while (hasArgs(2))
	{
		pop= nextArg();
		op2 = strx(nextArg());

		if (op2[0] == '@')
		{
			ans = strx(GetMemory(op2 + 1));
			memFree(op2);
			op2 = ans;
		}
	reopration:
		switch (m_tolower(*pop))
		{
		case '!':
			{
				char *swapop = op1;
				op1 = op2;
				op2 = swapop;
			}
			pop++;
			goto reopration;

		case '+': ans = calcLine(op1, '+', op2, radix, 0); break;
		case '-': ans = calcLine(op1, '-', op2, radix, 0); break;
		case '*': ans = calcLine(op1, '*', op2, radix, 0); break;
		case '/': ans = calcLine(op1, '/', op2, radix, basement); break;
		case 'm':
			{
				char *opd = calcLine(op1, '/', op2, radix, 0);
				char *opm = calcLine(opd, '*', op2, radix, 0);
				ans = calcLine(op1, '-', opm, radix, 0); // ������ op1 �Ɉ�v����B
				memFree(opd);
				memFree(opm);
			}
			break;

		case 'p': ans = calcPower(op1, toValue(op2), radix); break;
		case 'r': ans = calcRootPower(op1, toValue(op2), radix, basement); break;
		case 'l': ans = xcout("%u", calcLogarithm(op1, op2, radix)); break;

		case 'b': ans = strx(op1); basement = toValue(op2); break;
		case 'x':
			{
				uint newRadix = toValue(op2);
				ans = changeRadixCalcLine(op1, radix, newRadix, basement);
				radix = newRadix;
			}
			break;

		case '=': ans = strx(op1); SetMemory(op2, op1); break;
		case ';': ans = strx(op2); break;
		case ',': ans = InsSepOp(strx(op1), toValue(op2)); break;

		default:
			error();
		}
		cout("%s\n", ans);

		memFree(op1);
		memFree(op2);

		op1 = ans;
	}
	memFree(op1);

endFunc:
	FnlzMemory();
}
int main(int argc, char **argv)
{
	Main2();
	termination(0);
}
